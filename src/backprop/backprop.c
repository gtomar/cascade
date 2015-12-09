// The network will be trained with following NN specifications:
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#define _GNU_SOURCE

#define MAX_NO_OF_LAYERS 3 // 3 layers excluding the input layer: 784, 300, 100, 10
#define MAX_NO_OF_INPUTS 784 // the images are 28*28
#define MAX_NO_OF_NEURONS 410 // 300+100+10
#define MAX_NO_OF_WEIGHTS 784*300 + 300*100 + 100*10 // sum up weights between each layer pair
#define MAX_NO_OF_OUTPUTS 10 // the number of digits

void createNet( int, int *, int *, char *, double *, int );
void feedNetInputs(double *);
void updateNetOutput(void);
double *getOutputs();
void trainNet ( double, double, int, double * );
void applyBatchCumulations( double, double );
int loadNet(char *);
int saveNet(char *);
double getRand();
void getTrainSet();

struct neuron{
    double *output;
    double threshold;
    double oldThreshold;
    double batchCumulThresholdChange;
    char axonFamily;
    double *weights;
    double *oldWeights;
    double *netBatchCumulWeightChanges;
    int noOfInputs;
    double *inputs;
    double actFuncFlatness;
    double *error;
};

struct layer {
    int noOfNeurons;
    struct neuron * neurons;
};

static struct neuralNet {
    int noOfInputs;
    double *inputs;
    double *outputs;
    int noOfLayers;
    struct layer *layers;
    int noOfBatchChanges;
} theNet;

double getRand() {

    return (( (double)rand() * 2 ) / ( (double)RAND_MAX + 1 ) ) - 1;

}

static struct neuron netNeurons[MAX_NO_OF_NEURONS]; 			// array of all the neurons in the network
static double netInputs[MAX_NO_OF_INPUTS]; 						// array of all inputs
static double netNeuronOutputs[MAX_NO_OF_NEURONS]; 				// array of output of all neurons
static double netErrors[MAX_NO_OF_NEURONS]; 					// array of errors on each neuron
static struct layer netLayers[MAX_NO_OF_LAYERS]; 				// array of all layers in the network
static double netWeights[MAX_NO_OF_WEIGHTS]; 					// array of weights
static double netOldWeights[MAX_NO_OF_WEIGHTS]; 				// array of old weights
static double netBatchCumulWeightChanges[MAX_NO_OF_WEIGHTS]; 	// array of weight deltas for each weight

void createNet( int noOfLayers, int *noOfNeurons, int *noOfInputs, char *axonFamilies, double *actFuncFlatnesses, int initWeights ) {

    int i, j, counter, counter2, counter3, counter4;
    int totalNoOfNeurons, totalNoOfWeights;

    theNet.layers = netLayers;
    theNet.noOfLayers = noOfLayers;
    theNet.noOfInputs = noOfInputs[0];
    theNet.inputs = netInputs;

    totalNoOfNeurons = 0;
    for(i = 0; i < theNet.noOfLayers; i++) {
        totalNoOfNeurons += noOfNeurons[i];
    }
    for(i = 0; i < totalNoOfNeurons; i++) { netNeuronOutputs[i] = 0; }

    totalNoOfWeights = 0;
    for(i = 0; i < theNet.noOfLayers; i++) {
        totalNoOfWeights += noOfInputs[i] * noOfNeurons[i];
    }

    counter = counter2 = counter3 = counter4 = 0;
    for(i = 0; i < theNet.noOfLayers; i++) {
        for(j = 0; j < noOfNeurons[i]; j++) {
            if(i == theNet.noOfLayers-1 && j == 0) { // beginning of the output layer
                theNet.outputs = &netNeuronOutputs[counter];
            }
            netNeurons[counter].output = &netNeuronOutputs[counter]; // resume here, output is pointer to double
            netNeurons[counter].noOfInputs = noOfInputs[i];
            netNeurons[counter].weights = &netWeights[counter2];
            netNeurons[counter].netBatchCumulWeightChanges = &netBatchCumulWeightChanges[counter2];
            netNeurons[counter].oldWeights = &netOldWeights[counter2];
            netNeurons[counter].axonFamily = axonFamilies[i];
            netNeurons[counter].actFuncFlatness = actFuncFlatnesses[i];
            if ( i == 0) {
                netNeurons[counter].inputs = netInputs;
            }
            else {
                netNeurons[counter].inputs = &netNeuronOutputs[counter3];
            }
            netNeurons[counter].error = &netErrors[counter];
            counter2 += noOfInputs[i];
            counter++;
        }
        netLayers[i].noOfNeurons = noOfNeurons[i];
        netLayers[i].neurons = &netNeurons[counter4];
        if(i > 0) {
            counter3 += noOfNeurons[i-1];
        }
        counter4 += noOfNeurons[i];
    }

    // initialize weights and thresholds
     if ( initWeights == 1 ) {
        for( i = 0; i < totalNoOfNeurons; i++) { netNeurons[i].threshold = getRand(); }
        for( i = 0; i < totalNoOfWeights; i++) { netWeights[i] = getRand(); }
        for( i = 0; i < totalNoOfWeights; i++) { netOldWeights[i] = netWeights[i]; }
        for( i = 0; i < totalNoOfNeurons; i++) { netNeurons[i].oldThreshold = netNeurons[i].threshold; }
    }

    // initialize batch values
    for( i = 0; i < totalNoOfNeurons; i++) { netNeurons[i].batchCumulThresholdChange = 0; }
    for( i = 0; i < totalNoOfWeights; i++) { netBatchCumulWeightChanges[i] = 0; }
    theNet.noOfBatchChanges = 0;

}

void feedNetInputs(double *inputs) {
     int i;
     for ( i = 0; i < theNet.noOfInputs; i++ ) {
        netInputs[i] = inputs[i];
     }
}

static void updateNeuronOutput(struct neuron * myNeuron) {

    double activation = 0;
    int i;

    for ( i = 0; i < myNeuron->noOfInputs; i++) {
        activation += myNeuron->inputs[i] * myNeuron->weights[i];
    }
    activation += -1 * myNeuron->threshold;
    double temp;
    switch (myNeuron->axonFamily) {
        case 'g': // logistic
            temp = -activation / myNeuron->actFuncFlatness;
            /* avoid overflow */
            if ( temp > 45 ) {
                *(myNeuron->output) = 0;
            }
            else if ( temp < -45 ) {
                *(myNeuron->output) = 1;
            }
            else {
                *(myNeuron->output) = 1.0 / ( 1 + exp( temp ));
            }
            break;
        case 't': // tanh
            temp = -activation / myNeuron->actFuncFlatness;
            /* avoid overflow */
            if ( temp > 45 ) {
                *(myNeuron->output) = -1;
            }
            else if ( temp < -45 ) {
                *(myNeuron->output) = 1;
            }
            else {
                *(myNeuron->output) = ( 2.0 / ( 1 + exp( temp ) ) ) - 1;
            }
            break;
        case 'l': // linear
            *(myNeuron->output) = activation;
            break;
        default:
            break;
    }

}

void updateNetOutput( ) {

    int i, j;

    for(i = 0; i < theNet.noOfLayers; i++) {
        for( j = 0; j < theNet.layers[i].noOfNeurons; j++) {
            updateNeuronOutput(&(theNet.layers[i].neurons[j]));
        }
    }

}

static double derivative (struct neuron * myNeuron) {

    double temp;
    switch (myNeuron->axonFamily) {
        case 'g': // logistic
            temp = ( *(myNeuron->output) * ( 1.0 - *(myNeuron->output) ) ) / myNeuron->actFuncFlatness; break;
        case 't': // tanh
            temp = ( 1 - pow( *(myNeuron->output) , 2 ) ) / ( 2.0 * myNeuron->actFuncFlatness ); break;
        case 'l': // linear
            temp = 1; break;
        default:
            temp = 0; break;
    }
    return temp;

}

// learningRate and momentumRate will have no effect if batch mode is 'on'
void trainNet ( double learningRate, double momentumRate, int batch, double *outputTargets ) {

    int i,j,k;
    double temp;
    struct layer *currLayer, *nextLayer;

     // calculate errors
    for(i = theNet.noOfLayers - 1; i >= 0; i--) {
        currLayer = &theNet.layers[i];
        if ( i == theNet.noOfLayers - 1 ) { // output layer
            for ( j = 0; j < currLayer->noOfNeurons; j++ ) {
                *(currLayer->neurons[j].error) = derivative(&currLayer->neurons[j]) * ( outputTargets[j] - *(currLayer->neurons[j].output));
            }
        }
        else { // other layers
            nextLayer = &theNet.layers[i+1];
            for ( j = 0; j < currLayer->noOfNeurons; j++ ) {
                temp = 0;
                for ( k = 0; k < nextLayer->noOfNeurons; k++ ) {
                    temp += *(nextLayer->neurons[k].error) * nextLayer->neurons[k].weights[j];
                }
                *(currLayer->neurons[j].error) = derivative(&currLayer->neurons[j]) * temp;
            }
        }
    }

    // update weights n thresholds
    double tempWeight;
    for(i = theNet.noOfLayers - 1; i >= 0; i--) {
        currLayer = &theNet.layers[i];
        for ( j = 0; j < currLayer->noOfNeurons; j++ ) {

            // thresholds
            if ( batch == 1 ) {
                    currLayer->neurons[j].batchCumulThresholdChange += *(currLayer->neurons[j].error) * -1;
            }
            else {
                tempWeight = currLayer->neurons[j].threshold;
                currLayer->neurons[j].threshold += ( learningRate * *(currLayer->neurons[j].error) * -1 ) + ( momentumRate * ( currLayer->neurons[j].threshold - currLayer->neurons[j].oldThreshold ) );
                currLayer->neurons[j].oldThreshold = tempWeight;
            }

            // weights
            if ( batch == 1 ) {
                for( k = 0; k < currLayer->neurons[j].noOfInputs; k++ ) {
                    currLayer->neurons[j].netBatchCumulWeightChanges[k] +=  *(currLayer->neurons[j].error) * currLayer->neurons[j].inputs[k];
                }
            }
            else {
                for( k = 0; k < currLayer->neurons[j].noOfInputs; k++ ) {
                    tempWeight = currLayer->neurons[j].weights[k];
                    currLayer->neurons[j].weights[k] += ( learningRate * *(currLayer->neurons[j].error) * currLayer->neurons[j].inputs[k] ) + ( momentumRate * ( currLayer->neurons[j].weights[k] - currLayer->neurons[j].oldWeights[k] ) );
                    currLayer->neurons[j].oldWeights[k] = tempWeight;
                }
            }

        }
    }

    if(batch == 1) {
        theNet.noOfBatchChanges++;
    }

}

void applyBatchCumulations( double learningRate, double momentumRate ) {

    int i,j,k;
    struct layer *currLayer;
    double tempWeight;

    for(i = theNet.noOfLayers - 1; i >= 0; i--) {
        currLayer = &theNet.layers[i];
        for ( j = 0; j < currLayer->noOfNeurons; j++ ) {
            // thresholds
            tempWeight = currLayer->neurons[j].threshold;
            currLayer->neurons[j].threshold += ( learningRate * ( currLayer->neurons[j].batchCumulThresholdChange / theNet.noOfBatchChanges ) ) + ( momentumRate * ( currLayer->neurons[j].threshold - currLayer->neurons[j].oldThreshold ) );
            currLayer->neurons[j].oldThreshold = tempWeight;
            currLayer->neurons[j].batchCumulThresholdChange = 0;
            // weights
            for( k = 0; k < currLayer->neurons[j].noOfInputs; k++ ) {
                tempWeight = currLayer->neurons[j].weights[k];
                currLayer->neurons[j].weights[k] += ( learningRate * ( currLayer->neurons[j].netBatchCumulWeightChanges[k] / theNet.noOfBatchChanges ) ) + ( momentumRate * ( currLayer->neurons[j].weights[k] - currLayer->neurons[j].oldWeights[k] ) );
                currLayer->neurons[j].oldWeights[k] = tempWeight;
                currLayer->neurons[j].netBatchCumulWeightChanges[k] = 0;
            }
        }
    }

    theNet.noOfBatchChanges = 0;

}

double *getOutputs() {

    return theNet.outputs;

}

int loadNet(char *path) {

    int tempInt; double tempDouble; char tempChar;
    int i, j, k;

    int noOfLayers;
    int noOfNeurons[MAX_NO_OF_LAYERS];
    int noOfInputs[MAX_NO_OF_LAYERS];
    char axonFamilies[MAX_NO_OF_LAYERS];
    double actFuncFlatnesses[MAX_NO_OF_LAYERS];

    FILE *inFile;

    if(!(inFile = fopen(path, "rb")))
    return 1;

    fread(&tempInt,sizeof(int),1,inFile);
    noOfLayers = tempInt;

    for(i = 0; i < noOfLayers; i++) {

        fread(&tempInt,sizeof(int),1,inFile);
        noOfNeurons[i] = tempInt;

        fread(&tempInt,sizeof(int),1,inFile);
        noOfInputs[i] = tempInt;

        fread(&tempChar,sizeof(char),1,inFile);
        axonFamilies[i] = tempChar;

        fread(&tempDouble,sizeof(double),1,inFile);
        actFuncFlatnesses[i] = tempDouble;

    }

    createNet(noOfLayers, noOfNeurons, noOfInputs, axonFamilies, actFuncFlatnesses, 0);

    // now the weights
    for(i = 0; i < noOfLayers; i++) {
        for (j = 0; j < noOfNeurons[i]; j++) {
            fread(&tempDouble,sizeof(double),1,inFile);
            theNet.layers[i].neurons[j].threshold = tempDouble;
            for ( k = 0; k < noOfInputs[i]; k++ ) {
                fread(&tempDouble,sizeof(double),1,inFile);
                theNet.layers[i].neurons[j].weights[k] = tempDouble;
            }
        }
    }

    fclose(inFile);

    return 0;

}

void getTrainSet(double trainSetX[60000][784], double trainSetY[60000][1]) {

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int datum_counter = 0;
	fp = fopen("../../data/trainset","r");
	if (fp == NULL) exit(EXIT_FAILURE);
	while ((read = getline(&line, &len, fp)) != -1) {
		printf("train: reading datum:%d\n", datum_counter);
		char * pch;
		pch = strtok (line,",");
		int datum_point = 0;
		while (pch != NULL) {
			if (datum_point < 784) {
				trainSetX[datum_counter][datum_point] = atof(pch);
			} else {
				trainSetY[datum_counter][0] = atof(pch);
			}
			pch = strtok (NULL, ",");
			datum_point += 1;
		}
		datum_counter += 1;
	}
	fclose(fp);
	if (line) free(line);
}


void getTestSet(double testSetX[10000][784], double testSetY[10000][1]) {

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int datum_counter = 0;
	fp = fopen("../../data/testset","r");
	if (fp == NULL) exit(EXIT_FAILURE);
	while ((read = getline(&line, &len, fp)) != -1) {
		printf("test: reading datum:%d\n", datum_counter);
		char * pch;
		pch = strtok (line,",");
		int datum_point = 0;
		while (pch != NULL) {
			if (datum_point < 784) {
				testSetX[datum_counter][datum_point] = atoi(pch);
			} else {
				testSetY[datum_counter][0] = atoi(pch);
			}
			pch = strtok (NULL, ",");
			datum_point += 1;
		}
		datum_counter += 1;
	}
	fclose(fp);
	if (line) free(line);
}

int maxIndex(const double *arr, int length) {
    // returns the index of maximum value of array
    int i;
    double minimum = arr[0];
    int min_index = 0;
    for (i = 1; i < length; ++i) {
        if (minimum > arr[i]) {
            minimum = arr[i];
            min_index = i;
        }
    }
    return min_index;
}

int main() {

    double inputs[MAX_NO_OF_INPUTS];
    double outputTargets[MAX_NO_OF_OUTPUTS];



    /* determine layer parameters */
    int noOfLayers = 3; // input layer excluded
    int noOfNeurons[] = {300,100,10};
    int noOfInputs[] = {784,300,100};
    char axonFamilies[] = {'g','g','t'};
    double actFuncFlatnesses[] = {1,1,1};

    createNet(noOfLayers, noOfNeurons, noOfInputs, axonFamilies, actFuncFlatnesses, 1);

    // load the train data

    static double trainSetX[60000][784];
    static double trainSetY[60000][1];
    getTrainSet(trainSetX,trainSetY);

    // load the test data

    static double testSetX[10000][784];
    static double testSetY[10000][1];
    getTestSet(testSetX,testSetY);
    int epoch;
    for (epoch = 0; epoch < 10; epoch++) {

    	printf("epoch:%d\n",epoch);

    	int i;
    	    double tempTotal;
    	    int counter = 0;
    	    printf("training...\n");
    	    for(i = 0; i < 60000; i++) {

    	    	// get the i-th training instance and store it in the input vector

    	    	int j;
    	    	for (j = 0; j < 784; j++) {
    	    		inputs[j] = trainSetX[i][j];
    	    	}

    	        feedNetInputs(inputs);
    	        updateNetOutput();

    	        // make the outputTargets vector
    	        double outputTargets[10];
    	        int label = trainSetY[i][0];
    	        //int j;
    	        for (j = 0; j < 10; j++) {
    	        	if (label-1 == j) {
    	        		outputTargets[j] = 1.0;
    	        	} else {
    	        		outputTargets[j] = 0.0;
    	        	}
    	        }

    	        /* train using batch training ( don't update weights, just cumulate them ) */
    	        //printf("training input:%d\n",i);
    	        trainNet(0.01, 0, 1, outputTargets);
    	        counter++;
    	        /* apply batch changes after 1000 loops use .8 learning rate and .8 momentum */
    	        if(counter == 100) { applyBatchCumulations(.8,.8); counter = 0;}
    	    }

    	    /* test it */
    	    double *outputs;
    	    int correct = 0;
    	    int incorrect = 0;

    	    for(i = 0; i < 10000; i++) {

    	    	// make the input vector
    	    	int j;
    	    	for (j = 0; j < 784; j++) {
    	    		inputs[j] = testSetX[i][j];
    	    	}
    	    	//printf("testing input:%d\n",i);
    	        feedNetInputs(inputs);
    	        updateNetOutput();
    	        outputs = getOutputs();

    	        // find the highest from the output and give that as the answer
    	        int NNOutput = maxIndex(outputs,10);
    	        int trueLabel = testSetY[i][0];
    	        if (NNOutput == trueLabel) {
    	        	correct++;
    	        } else {
    	        	incorrect++;
    	        }
    	    }
    	    printf("accuracy:%f\n",(float)correct/(correct+incorrect));
    }


    return 0;

}
