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
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

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
int **weights;
int num_weights;
int threshold;
int hist_len;
uint64_t perceptron_hist;

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
  switch(bpType) {
    case CUSTOM:
      threshold = 1.93 * hist_len + 14;
      perceptron_hist = 0;

      weights = (int **)malloc(num_weights * sizeof(int *));
      for(int i = 0; i < num_weights; i++) {
        weights[i] = (int *)malloc((hist_len + 1) * sizeof(int));
        for(int j = 0; j < hist_len + 1; j++) {
          weights[i][j] = NOTTAKEN;
        }
      }

      break;
  }
}

int get_perceptron_res(uint32_t pc) {
  int index = (pc >> 2) % num_weights;
  int res = weights[index][0];
  int i = 0;
  int pos_or_neg;
  while(i < hist_len + 1) {
    pos_or_neg = (perceptron_hist & (1 << i)) != 0 ? 1 : -1;
    res += (pos_or_neg * weights[index][i]);
    i++;
  }

  return res;
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

  int res = get_perceptron_res(pc);

  // Make a prediction based on the bpType
  switch (bpType) {
    case CUSTOM:
      if(res > 0)
        return TAKEN;
      return NOTTAKEN;

    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome) {
  int index = (pc >> 2) % num_weights;
  int res = get_perceptron_res(pc);
  int sgn;
  switch(bpType) {
    case CUSTOM:
      if(outcome == NOTTAKEN)
        sgn = -1;
      else
        sgn = 1;

      if((res > 0 && outcome == NOTTAKEN) || (abs(res) < threshold)) {
        weights[index][0] += sgn;
        for(int i = 1; i < hist_len + 1; i++) {
          weights[index][i] = (perceptron_hist & (1 << i)) != 0 ? weights[index][i] + sgn : weights[index][i] - sgn;
        }
      }

      perceptron_hist = (perceptron_hist << 1) | outcome;
      perceptron_hist &= (1 << hist_len) - 1;
      break;
    default:
      break;
  }
}
