//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Priyanka Bhatia, Rahul Dalal";
const char *studentID   = "A59010880, A59005007";
const char *email       = "pbhatia@ucsd.edu, rdalal@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
uint8_t *globalPredictorTable;
uint8_t *localPredictorTable;
uint8_t *localHistoryTable;
uint8_t *choicePredictorTable;
uint64_t phist;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  int globalEntries = 1 << ghistoryBits;
	int localPredictorEntries = 1 << lhistoryBits;
	int localHistoryEntries = 1 << pcIndexBits;

  phist=0;
  globalPredictorTable = (uint8_t*)malloc(globalEntries * sizeof(uint8_t));
	localPredictorTable = (uint8_t*)malloc(localPredictorEntries * sizeof(uint8_t));
	localHistoryTable = (uint8_t*)malloc(localHistoryEntries * sizeof(uint8_t));
  choicePredictorTable = (uint8_t*)malloc(globalEntries * sizeof(uint8_t));

  int i=0;
  while(i < globalEntries){
    globalPredictorTable[i] = WN;
    choicePredictorTable[i] = WT;
    i++;
  }

  int j = 0;
  while(j < localPredictorEntries){
    localPredictorTable[j] = WN;
    j++;
  }

  int k = 0;
  while(k < localHistoryEntries){
    localHistoryTable[k] = SN;
    k++;
  }

	
}

//find choice
uint8_t
searchChoice(uint32_t pc){
    uint32_t values = 1<<pcIndexBits;
    uint32_t pcLower = pc & (values-1);
    uint32_t localValue = localPredictorTable[localHistoryTable[pcLower]];
    uint32_t globalValue = globalPredictorTable[phist];

    uint32_t globalPrediction = NOTTAKEN;
    uint32_t localPrediction = NOTTAKEN;

    //search for global prediction
    if(globalValue == WN || globalValue == SN){
        globalPrediction = NOTTAKEN;
    }else if(globalValue == WT || globalValue == ST){
        globalPrediction = TAKEN;
    }else{
        printf("Error searching for global prediction value");
    }

    //search for local prediction
    if(localValue == WN || localValue == SN){
        localPrediction = NOTTAKEN;
    }else if(localValue == WT || localValue == ST){
        localPrediction = TAKEN;
    }else{
        printf("Error searching for local prediction value");
    }

    uint32_t choiceValue = (globalPrediction << 1) | localPrediction;
    return choiceValue;
}


// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  if(bpType == STATIC){
    return TAKEN;
  }else if(bpType == TOURNAMENT){
      //find choice
      uint32_t choiceValue = searchChoice(pc);

      //make predictions
      if(choiceValue == WN){
          if(choicePredictorTable[phist] > 1){
            return NOTTAKEN;
          }else{
            return TAKEN;
          }
      }else if(choiceValue == SN){
          return NOTTAKEN;
      }else if(choiceValue == WT){
          if(choicePredictorTable[phist] > 1){
            return TAKEN;
          }else{
            return NOTTAKEN;
          }
      }else if(choiceValue == ST){
          return TAKEN;
      }

      // printf("error from make prediction \n");
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  uint32_t choiceValues = searchChoice(pc);
	uint32_t values = 1<<pcIndexBits;
	uint32_t pcLower = pc & (values-1);
	int globalEntries = 1 << ghistoryBits;
	int localHistoryEntries = 1 << pcIndexBits;

  //check outcome
  switch(outcome){
    case TAKEN:
      if(localPredictorTable[localHistoryTable[pcLower]] < 3){
          localPredictorTable[localHistoryTable[pcLower]] += 1;
      }

      if(globalPredictorTable[phist] < 3){
          globalPredictorTable[phist] += 1;
      }

      break;
    case NOTTAKEN:
      if(localPredictorTable[localHistoryTable[pcLower]] > 0){
          localPredictorTable[localHistoryTable[pcLower]] -= 1;
      }

      if(globalPredictorTable[phist] > 0){
          globalPredictorTable[phist] -= 1;
      }

      break;
  }

  localHistoryTable[pcLower]=(localHistoryTable[pcLower]<<1) | outcome;
	localHistoryTable[pcLower]= localHistoryTable[pcLower] & (localHistoryEntries-1);

  int predictedChoice = choicePredictorTable[phist];
  if(choiceValues == WN){
      if(outcome == 1){
          if(predictedChoice > 0){
              choicePredictorTable[phist] = predictedChoice - 1;
          }else{
              choicePredictorTable[phist] = predictedChoice;
          }
      }else{
          if(predictedChoice < 3){
              choicePredictorTable[phist] = predictedChoice + 1;
          }else{
              choicePredictorTable[phist] = predictedChoice;
          }
      }
  }else if(choiceValues == WT){
      if(outcome == 1){
          if(predictedChoice < 3){
              choicePredictorTable[phist] = predictedChoice + 1;
          }else{
              choicePredictorTable[phist] = predictedChoice;
          }
      }else{
          if(predictedChoice > 0){
              choicePredictorTable[phist] = predictedChoice - 1;
          }else{
              choicePredictorTable[phist] = predictedChoice;
          }
      }
  }

  phist = (phist << 1) | outcome;
  phist = phist & (globalEntries - 1);

}
