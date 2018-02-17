#ifndef FluidTranCntrl_H
#define FluidTranCntrl_H
#include "ValveCntrl.h"
#include "PumpCntrl.h"
#include "HeatCntrl.h"
#include "tempMeasurement.h"
#include "weightMeasurement.h"
#include "TimerAPI.h"

class FluidTranCntrl{
public:
	int fluidid[5];
	int beakers[4];
	HeatCntrl lineHeat[]; // number of heatings on tubes TBD
	tempMeasurement tempSensors[4];
	ValveCntrl valves[10];
	PumpCntrl pumps[6];
  weightMeasurement scales[3];

	// Constructor
	FluidTranCntrl();
	// Temperature measurement
	double getTemp(int sensorID);
	// Transferring Process
	int activateTransfer();
	// Cleaning Process
	int cleaningTransfer();
	// Set the temperature
	int setTemp(int sensorID);
	// Set the pump power
	int setPump(int pumpID);
	// Measure the weight
	double getWeightTransferred(int scaleID);
	// Terminate Transferring
	int terminateTransfer();
	// Open openValves
	void openValves(void* ID);

};
#endif /* FluidTranCntrl_H */
