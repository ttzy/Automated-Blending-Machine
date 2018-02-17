#include "FluidTranCntrl.h"
#define Kp
#define Ki
#define Kd
extern "C" const int PROPORTIONS[5] = {/* proportion of dispensing materials */}
/* lineHeat */
#define TEMP
/* lineheat */
#define TIME_HOLD
/* For dispensing */
#define MAX_TIME
/* For Timer */
#define DELAY

FluidTranCntrl::FluidTranCntrl() {
  fluidid[5] = {/* Fluid ID for five fluids */};
  tempSensors[4] = {tempSensor(IRTEMP1_PIN), tempSensor(IRTEMP2_PIN), tempSensor(IRTEMP3_PIN), tempSensor(IRTEMP4_PIN)};
  beakers[4] =  {BEAKER1_POS, BEAKER2_POS, BEAKER3_POS, GARBAGE};
  valves[10] = { Valve1(VALVE_1), Valve1(VALVE_2), Valve1(VALVE_3), Valve1(VALVE_4),
                Valve1(VALVE_5), Valve1(VALVE_6), Valve1(VALVE_7), Valve1(VALVE_8),
                Valve1(VALVE_9), Valve1(VALVE_10) };
  pumps[6] = { Pump1(PUMP_1), Pump2(PUMP_2), Pump3(PUMP_3), Pump4(PUMP_4), Pump5(PUMP_5),
                Pump6(PUMP_6) };
  scales[3] = { Scales(LOAD_CELL_1), Scales(LOAD_CELL_2), Scales(LOAD_CELL_3) };
}

double FluidTranCntrl::getTemp(int sensorID) {
  // Sensor ID is the index of temperature sensor in the array
  return tempSensors[sensorID].measureheat();
}

/*
  valve[0] - pump[0] - Material 0 - valve[5]
  valve[1] - pump[1] - Material 1 - valve[6]
  valve[2] - pump[2] - Material 2 - valve[7]
  valve[3] - pump[3] - Material 3 - valve[8]
  valve[4] - pump[4] - Material 4 - valve[9]
*/
int FluidTranCntrl::activateTransfer() {
  INT8 name = 7;
  INT8U err;
  INT8U perr;
  double error = 0.0;
  double t = 0.0;
  double integral = 0.0;
  double derivative = 0.0;
  // Three beakers
  for (int j = 0; j < 3; ++j) {
    // Five dispensing valves and five raw matierials valves
    for (int i = 0; i < 5; ++i) {
      valves[i].moveValue(beakers[j]);
      /* setTemp(lineHeatID); turn on the tube heater which corresponds to the tube that is dispensing. */
      while(1) {
        // Measure the current weight
        current_weight = getWeightTransferred();
        // Calculate the previous error.
        previous_error = error;
        // Calculate the current error
        error = PROPORTIONS[i] - current_weight;
        // Calculate the integral
        integral = integral + error;
        // Calculate the derivative
        derivative = error - previous_error;
        // Calculate the control variable
        t = Kp * error + Ki * integral + Kd * derivative; // define Kp, Ki, Kd
        /* Set limits and end conditions for control variable */
        if (t > MAX_TIME) {
          /* Error is too large. Increase the pump power */
          setPump(i);
          t = MAX_TIME;
        }
        // open the valves, open the pumps
        // Let the fluid transfer for certain amount of time
        if (t > 0) {
          int ID = i;
          RTOS_TMR *Tmr = RTOSTmrCreate(t, 0, RTOS_TMR_ONE_SHOT, openValves, &ID, &name, &err);
          RTOSTmrStart(Tmr, &perr);
        }
        // close the valves, close the pumps
        valves[i + 5].closeValue();
        pumps[i].closeValue();
        valves[i].closeValue();
        RTOSTmrDel(Tmr, &perr);
        if (t <= 0)
          break;
      }
    }
  }
  return 1;
}

void FluidTranCntrl::openValves(void* ID) {
  int* valveNumber = (int*) ID;
  valves[(*valveNumber) + 5].openValue();
  valves[*valveNumber].openValue();
  pumps[*valveNumber].openValue();
}

void FluidTranCntrl::cleaningValves(void* ID) {
  int* valveNumber = (int*) ID;
  valves[(*valveNumber) + 10].openValue();
  pumps[5].openValue();
  valves[*valveNumber].openValue();
}
/*
  Cleaning process adds additional five valves !!!!
  valve[10] - cleaning material 1 - valve[0]
  valve[11] - cleaning material 2 - valve[1]
  valve[12] - cleaning material 3 - valve[2]
  valve[13] - cleaning material 4 - valve[3]
  valve[14] - cleaning material 5 - valve[4]
  pump[5]
*/
int FluidTranCntrl::cleaningTransfer() {
  INT8 name = 17;
  INT8U err;
  INT8U perr;
  for (int i = 0; i < 5; ++i) {
    valves[i].moveValve(beakers[4]);
    int ID = i;
    RTOS_TMR *Tmr = RTOSTmrCreate(DELAY, 0, RTOS_TMR_ONE_SHOT, openValves, &ID, &name, &err);
    RTOSTmrStart(Tmr, &perr);
    pumps[5].closeValue();
    valves[i].closeValue();
    valves[i + 10].closeValue();
    RTOSTmrDel(Tmr, &perr);
  }
}

double FluidTranCntrl::getWeightTransferred(int scaleID) {
  return scales[scaleID].measureweight();
}

int FluidTranCntrl::setTemp(int lineHeatID) {
  /* Heat the tubes to certain temperature */
  lineHeat[lineHeatID].setDesiredTemp(TEMP, TIME_HOLD);
  return 1;
}

int FluidTranCntrl::setPump(int pumpID) {
  /* Turn up the pump power */
  pumps[pumpID].turnup();
  return 1;
}

int FluidTranCntrl::terminateTransfer() {
  for (int i = 0; i < 10; ++i)
    valves[i].closeValue();
  for (int i = 0; i < 3; ++i)
    pumps[i].closeValue();
  return 1;
}
