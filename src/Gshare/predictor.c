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
uint32_t ghist;
uint8_t *bhtgshare;

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
  int entries = 1 << ghistoryBits;
  bhtgshare = (uint8_t*)malloc(entries * sizeof(uint8_t));
  int i = 0;
  ghist = 0;
  while(i < entries){
    bhtgshare[i] = WN;
    i++;
  }
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
  }else if(bpType == GSHARE){
      uint32_t masking = UINT32_MAX >> (32 - ghistoryBits);
      uint32_t pcLower = pc & masking;
      uint32_t ghistLower = ghist & masking;
      uint32_t index =  ghistLower ^ pcLower;

      if(bhtgshare[index] == SN || bhtgshare[index] == WN){
        return NOTTAKEN;
      }else if(bhtgshare[index] == ST || bhtgshare[index] == WT){
        return TAKEN;
      }

      printf("error");
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
  uint32_t masking = UINT32_MAX >> (32 - ghistoryBits);
  uint32_t pcLower = pc & masking;
  uint32_t ghistLower = ghist & masking;
  uint32_t index = ghistLower ^ pcLower;

  if(bhtgshare[index] == WN){
      if(outcome == TAKEN){
          bhtgshare[index] = WT;
      }else{
          bhtgshare[index] = SN;
      }
  }else if(bhtgshare[index] == SN){
      if(outcome == TAKEN){
          bhtgshare[index] = WN;
      }else{
          bhtgshare[index] = SN;
      }
  }else if(bhtgshare[index] == WT){
      if(outcome == TAKEN){
          bhtgshare[index] = ST;
      }else{
          bhtgshare[index] = WN;
      }
  }else if(bhtgshare[index] == ST){
      if(outcome == TAKEN){
          bhtgshare[index] = ST;
      }else{
          bhtgshare[index] = WT;
      }
  }else{
          printf("error");
  }

  ghist = ((ghist << 1) | outcome); 
}
