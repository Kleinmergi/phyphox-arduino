#if defined(ESP32) && !defined(ARDUINO_SENSEBOX_MCU_ESP32S2)
#include "phyphoxBLE_common.h"
#include "phyphoxBLE_ESP32.h"
#include "Arduino.h"
#include <stdio.h>
#include "esp_system.h"
//#define DEBUG
//init statics
uint8_t PhyphoxBLE::data_package[20] = {0};
void (*PhyphoxBLE::configHandler)() = nullptr;
void (*PhyphoxBLE::experimentEventHandler)() = nullptr;
uint8_t storage[16000];
//uint8_t *storage = (uint8_t*) malloc(8000 * sizeof(char));
char *PhyphoxBLE::EXPARRAY=(char*)storage;
uint8_t* PhyphoxBLE::p_exp = nullptr;
size_t PhyphoxBLE::expLen = 0;
HardwareSerial* PhyphoxBLE::printer =nullptr;

BLEServer *PhyphoxBLE::myServer;
BLEService *PhyphoxBLE::phyphoxExperimentService;
BLEService *PhyphoxBLE::phyphoxDataService;
BLEDescriptor *PhyphoxBLE::myExperimentDescriptor;
BLEDescriptor *PhyphoxBLE::myDataDescriptor;
BLEDescriptor *PhyphoxBLE::myEventDescriptor;
BLEDescriptor *PhyphoxBLE::myConfigDescriptor;
BLECharacteristic *PhyphoxBLE::dataCharacteristic;
BLECharacteristic *PhyphoxBLE::eventCharacteristic;
BLECharacteristic *PhyphoxBLE::experimentCharacteristic;
BLECharacteristic *PhyphoxBLE::configCharacteristic;
BLEAdvertising *PhyphoxBLE::myAdvertising;
TaskHandle_t PhyphoxBLE::TaskTransfer;
uint8_t* PhyphoxBLE::data;

uint16_t PhyphoxBLE::minConInterval = 6;  //7.5ms
uint16_t PhyphoxBLE::maxConInterval = 24; //30ms
uint16_t PhyphoxBLE::slaveLatency = 0;
uint16_t PhyphoxBLE::timeout = 50;
uint16_t PhyphoxBLE::currentConnections=0;
bool     PhyphoxBLE::isSubscribed=false;

uint8_t PhyphoxBLE::eventData[17]={0};
int64_t PhyphoxBLE::experimentTime = 0;
int64_t PhyphoxBLE::systemTime = 0;
uint8_t PhyphoxBLE::eventType = 0;
uint16_t PhyphoxBLE::MTU = 20;
uint16_t PhyphoxBleExperiment::MTU = 20;

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
class MyExpCallback: public BLECharacteristicCallbacks {
    private:
      HardwareSerial* printer;

    public:
      MyExpCallback(HardwareSerial* hwPrint) {
        printer = hwPrint;
      }
    private:

    void onSubscribe(BLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue) override {
      #ifdef DEBUG
        if(printer){
          printer->println("onSubscribe (exp)");
        }
      #endif

      if (subValue) {
        // start when_subscription_received() on cpu 1
        PhyphoxBLE::startTask();
      }
    };
  };
#else
class MyExpDescCallback: public BLEDescriptorCallbacks {
    private:
      HardwareSerial* printer;

    public:
      MyExpDescCallback(HardwareSerial* hwPrint) {
        printer = hwPrint;
      }
    private:

    void onWrite(BLEDescriptor* pDescriptor) override {
      #ifdef DEBUG
        if(printer){
          printer->println("descriptor write (exp)");
        }
      #endif
      uint8_t* rxValue = pDescriptor->getValue();

      if(pDescriptor->getLength() > 0){
        if (rxValue[0] == 1) {
          // start when_subscription_received() on cpu 1
          PhyphoxBLE::startTask();
        }
      }
    };
  };
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
class MyDataCallback: public BLECharacteristicCallbacks {
    private:
      HardwareSerial* printer;

    public:
      MyDataCallback(HardwareSerial* hwPrint) {
        printer = hwPrint;
      }

    private:

    void onSubscribe(BLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue) override {
      #ifdef DEBUG
        if(printer){
          printer->println("onSubscribe (data)");
        }
      #endif

      if(subValue > 0){
        PhyphoxBLE::isSubscribed=true;
      }
    };
  };
#else
class MyDataDescCallback: public BLEDescriptorCallbacks {
    private:
      HardwareSerial* printer;

    public:
      MyDataDescCallback(HardwareSerial* hwPrint) {
        printer = hwPrint;
      }

    private:

    void onWrite(BLEDescriptor* pDescriptor) override {
      #ifdef DEBUG
        if(printer){
          printer->println("descriptor write (data)");
        }
      #endif

      if(pDescriptor->getLength() > 0){
        PhyphoxBLE::isSubscribed=true;
      }
    };
  };
#endif

class MyEventCallback: public BLECharacteristicCallbacks {
  private:
    HardwareSerial* printer;

  public:
    MyEventCallback(HardwareSerial* hwPrint) {
        printer = hwPrint;
    }
    private:

    void onWrite(BLECharacteristic *pCharacteristic) override {
      #ifdef DEBUG
        if(printer){
          printer->println("characteristic write (event)");
        }
      #endif
      PhyphoxBLE::eventCharacteristicHandler();
    };
  };

class MyCharCallback: public BLECharacteristicCallbacks {
  private:
    HardwareSerial* printer;

  public:
    MyCharCallback(HardwareSerial* hwPrint) {
        printer = hwPrint;
    }
  private:
    PhyphoxBLE* myServerPointer;
    void onWrite(BLECharacteristic *pCharacteristic) override {
      #ifdef DEBUG
        if(printer){
          printer->println("characteristic write (config)");
        }
      #endif
      PhyphoxBLE::configHandlerDebug();
    }
};

class MyServerCallbacks: public BLEServerCallbacks {

private:
    HardwareSerial* printer;

public:
    MyServerCallbacks(HardwareSerial* hwPrint) {
        printer = hwPrint;
    }

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
    void onConnect(BLEServer *pServer, ble_gap_conn_desc *desc) override {
      #ifdef DEBUG
        if(printer){
          printer->println("onConnect (NimBLE)");
        }
      #endif
      pServer->updateConnParams(desc->conn_handle, PhyphoxBLE::minConInterval, PhyphoxBLE::maxConInterval, PhyphoxBLE::slaveLatency, PhyphoxBLE::timeout);
      PhyphoxBLE::currentConnections+=1;
    };
#else
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override {
      #ifdef DEBUG
        if(printer){
          printer->println("onConnect (legacy)");
        }
      #endif
      pServer->updateConnParams(param->connect.remote_bda,PhyphoxBLE::minConInterval,PhyphoxBLE::maxConInterval,PhyphoxBLE::slaveLatency,PhyphoxBLE::timeout);
      PhyphoxBLE::currentConnections+=1;
    };
#endif

    void onDisconnect(BLEServer* pServer) override {
      PhyphoxBLE::disconnected();
      PhyphoxBLE::currentConnections-=1;
    }


};

void PhyphoxBLE::configHandlerDebug(){
  if(configHandler!=nullptr){
    (*configHandler)();
  }
}

void PhyphoxBLE::eventCharacteristicHandler(){
  uint8_t* data = eventCharacteristic->getData();
  memcpy(&eventData[0],data,17);
  int64_t et,st;
  memcpy(&et,data+1,8);
  memcpy(&st,data+1+8,8);
  PhyphoxBLE::eventType = eventData[0];
  PhyphoxBLE::systemTime = swap_int64(st);
  PhyphoxBLE::experimentTime = swap_int64(et);
  if(experimentEventHandler!=nullptr){
    (*experimentEventHandler)();
  }
}

void PhyphoxBLE::setMTU(uint16_t mtuSize) {
    BLEDevice::setMTU(mtuSize+3); //user mtu size + 3 for overhead

    PhyphoxBLE::MTU = mtuSize;
    PhyphoxBleExperiment::MTU = mtuSize;
}

void PhyphoxBLE::start(const char* DEVICE_NAME, uint8_t* exp_pointer, size_t len){
  p_exp = exp_pointer;
  expLen = len;
  start(DEVICE_NAME);
}

void PhyphoxBLE::start(uint8_t* exp_pointer, size_t len){
  p_exp = exp_pointer;
  expLen = len;
  start();
}

void PhyphoxBLE::start(const char * DEVICE_NAME)
{
  deviceName = DEVICE_NAME;
  #ifdef DEBUG
    if(printer){
      printer->println("starting server");
    }
  #endif
	if(p_exp == nullptr){
          PhyphoxBleExperiment defaultExperiment;

          //View
          PhyphoxBleExperiment::View firstView;

          //Graph
          PhyphoxBleExperiment::Graph firstGraph;      //Create graph which will plot random numbers over time
          firstGraph.setChannel(0,1);

          //Value
          PhyphoxBleExperiment::Value valueField;
          valueField.setChannel(1);

          firstView.addElement(firstGraph);
          firstView.addElement(valueField);
          defaultExperiment.addView(firstView);

          addExperiment(defaultExperiment);
    }

	BLEDevice::init(DEVICE_NAME);
	myServer = BLEDevice::createServer();
  myServer->setCallbacks(new MyServerCallbacks(printer));
	phyphoxExperimentService = myServer->createService(phyphoxBleExperimentServiceUUID);

  experimentCharacteristic = phyphoxExperimentService->createCharacteristic(
          phyphoxBleExperimentCharacteristicUUID,
          BLECharacteristic::PROPERTY_READ   |
           BLECharacteristic::PROPERTY_WRITE |
           BLECharacteristic::PROPERTY_NOTIFY
      );
  eventCharacteristic = phyphoxExperimentService->createCharacteristic(
          phyphoxBleEventCharacteristicUUID,
          BLECharacteristic::PROPERTY_WRITE
      );

  phyphoxDataService = myServer->createService(phyphoxBleDataServiceUUID);

	dataCharacteristic = phyphoxDataService->createCharacteristic(
	     phyphoxBleDataCharacteristicUUID,
	     BLECharacteristic::PROPERTY_READ |
	     BLECharacteristic::PROPERTY_WRITE |
	     BLECharacteristic::PROPERTY_NOTIFY

	   );

    configCharacteristic = phyphoxDataService->createCharacteristic(
          phyphoxBleConfigCharacteristicUUID,
          BLECharacteristic::PROPERTY_READ   |
           BLECharacteristic::PROPERTY_WRITE |
           BLECharacteristic::PROPERTY_NOTIFY
      );

  myExperimentDescriptor = new BLE2902();
  myDataDescriptor = new BLE2902();
  myEventDescriptor = new BLE2902();
  myConfigDescriptor = new BLE2902();

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
  experimentCharacteristic->setCallbacks(new MyExpCallback(printer));
  dataCharacteristic->setCallbacks(new MyDataCallback(printer));
#else
  myExperimentDescriptor->setCallbacks(new MyExpDescCallback(printer));
  myDataDescriptor->setCallbacks(new MyDataDescCallback(printer));
#endif
  eventCharacteristic->setCallbacks(new MyEventCallback(printer));
  configCharacteristic->setCallbacks(new MyCharCallback(printer));

  dataCharacteristic->addDescriptor(myDataDescriptor);
  experimentCharacteristic->addDescriptor(myExperimentDescriptor);
  eventCharacteristic->addDescriptor(myEventDescriptor);
  configCharacteristic->addDescriptor(myConfigDescriptor);


  phyphoxExperimentService->start();
  phyphoxDataService->start();
  myAdvertising = BLEDevice::getAdvertising();
  myAdvertising->addServiceUUID(phyphoxExperimentService->getUUID());
  BLEDevice::startAdvertising();

}

void PhyphoxBLE::start() {
    PhyphoxBLE::start("phyphox-Arduino");
}

void PhyphoxBLE::poll() {
}

void PhyphoxBLE::poll(int timeout) {
}

//thank you stackoverflow =)
void PhyphoxBLE::staticStartTask(void* _this){
	PhyphoxBLE::when_subscription_received();
  delay(1);
}

void PhyphoxBLE::startTask()
{
	xTaskCreatePinnedToCore(staticStartTask, "TaskTransfer",10000, NULL,1, &TaskTransfer, tskNO_AFFINITY);
  delay(1);
}

void PhyphoxBLE::write(float& value)
{
  /**
   * \brief Write a single float into characteristic
     * The float is parsed to uint8_t*
     * because the gattServer write method
     * expects a pointer to uint8_t
     * \param f1 represent a float most likeley sensor data
    */
  data = reinterpret_cast<uint8_t*>(&value);
  dataCharacteristic->setValue(data,4);
  dataCharacteristic->notify();
}

void PhyphoxBLE::write(float& f1, float& f2)
{
  float array[2] = {f1, f2};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic->setValue(data,8);
  dataCharacteristic->notify();
}
void PhyphoxBLE::write(float& f1, float& f2, float& f3)
{
  float array[3] = {f1, f2, f3};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic->setValue(data,12);
  dataCharacteristic->notify();
}
void PhyphoxBLE::write(float& f1, float& f2, float& f3, float& f4)
{
  float array[4] = {f1, f2, f3, f4};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic->setValue(data,16);
  dataCharacteristic->notify();
}
void PhyphoxBLE::write(float& f1, float& f2, float& f3, float& f4, float& f5)
{
  float array[5] = {f1, f2, f3, f4, f5};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic->setValue(data,20);
  dataCharacteristic->notify();
}
void PhyphoxBLE::write(uint8_t *arrayPointer, unsigned int arraySize)
{
  dataCharacteristic->setValue(arrayPointer,arraySize);
  dataCharacteristic->notify();
}
void PhyphoxBLE::write(float *arrayPointer, unsigned int arrayLength)
{
  uint8_t dataBuffer[arrayLength*4];
  memcpy(&dataBuffer[0], &arrayPointer[0],arrayLength*4);
  dataCharacteristic->setValue(&dataBuffer[0],arrayLength*4);
  dataCharacteristic->notify();
}


void PhyphoxBLE::read(uint8_t *arrayPointer, unsigned int arraySize)
{
  uint8_t* data = configCharacteristic->getData();
  memcpy(arrayPointer,data,arraySize);
}

void PhyphoxBLE::read(float& f)
{
  uint8_t* data = configCharacteristic->getData();
  memcpy(&f,data,4);
}
void PhyphoxBLE::read(float& f1, float& f2)
{
  uint8_t* data = configCharacteristic->getData();
  memcpy(&f1,data,4);
  memcpy(&f2,data+4,4);
}
void PhyphoxBLE::read(float& f1, float& f2, float& f3)
{
  uint8_t* data = configCharacteristic->getData();
  memcpy(&f1,data,4);
  memcpy(&f2,data+4,4);
  memcpy(&f3,data+8,4);
}
void PhyphoxBLE::read(float& f1, float& f2, float& f3, float& f4)
{
  uint8_t* data = configCharacteristic->getData();
  memcpy(&f1,data,4);
  memcpy(&f2,data+4,4);
  memcpy(&f3,data+8,4);
  memcpy(&f4,data+12,4);
}
void PhyphoxBLE::read(float& f1, float& f2, float& f3, float& f4, float& f5)
{
  uint8_t* data = configCharacteristic->getData();
  memcpy(&f1,data,4);
  memcpy(&f2,data+4,4);
  memcpy(&f3,data+8,4);
  memcpy(&f4,data+12,4);
  memcpy(&f5,data+16,4);
}


void PhyphoxBLE::when_subscription_received()
{

    myAdvertising->stop();

    uint8_t* exp = p_exp;
    size_t exp_len = expLen;

    uint8_t header[20] = {0}; //20 byte as standard package size for ble transfer
    const char phyphox[] = "phyphox";
    uint32_t table[256];
    phyphoxBleCrc32::generate_table(table);
    uint32_t checksum = phyphoxBleCrc32::update(table, 0, exp, exp_len);
    size_t arrayLength = exp_len;
    uint8_t experimentSizeArray[4] = {0};
    experimentSizeArray[0]=  (arrayLength >> 24);
    experimentSizeArray[1]=  (arrayLength >> 16);
    experimentSizeArray[2]=  (arrayLength >> 8);
    experimentSizeArray[3]=  arrayLength;

    uint8_t checksumArray[4] = {0};
    checksumArray[0]= (checksum >> 24) & 0xFF;
    checksumArray[1]= (checksum >> 16) & 0xFF;
    checksumArray[2]= (checksum >> 8) & 0xFF;
    checksumArray[3]= checksum & 0xFF;

    copy(phyphox, phyphox+7, header);
    copy(experimentSizeArray, experimentSizeArray+ 4, header + 7);
    copy(checksumArray, checksumArray +  4, header +11);
    experimentCharacteristic->setValue(header,sizeof(header));
    experimentCharacteristic->notify();

    for(size_t i = 0; i < exp_len/20; ++i){
        copy(exp+i*20, exp+i*20+20, header);
        experimentCharacteristic->setValue(header,sizeof(header));
        experimentCharacteristic->notify();
		    delay(10);// mh does not work anymore with 1ms delay?!
	}

	if(exp_len%20 != 0){
		const size_t rest = exp_len%20;
		uint8_t slice[rest];
		copy(exp + exp_len - rest, exp + exp_len, slice);
		experimentCharacteristic->setValue(slice,sizeof(slice));
		experimentCharacteristic->notify();
		delay(1);
	}


	myAdvertising->start();


	vTaskDelete( NULL );

}
void PhyphoxBLE::addExperiment(PhyphoxBleExperiment& exp)
{
  for (int i = 0; i < 16000; i++)
  {
    storage[i]=0;
  }

  exp.getFirstBytes(EXPARRAY, deviceName);
	for(uint8_t i=0;i<phyphoxBleNViews; i++){
		for(int j=0; j<phyphoxBleNElements; j++){
			exp.getViewBytes(EXPARRAY,i,j);
		}
	}
	exp.getLastBytes(EXPARRAY);
	p_exp =  (uint8_t*)&EXPARRAY[0];
	expLen = strlen(EXPARRAY);
}

void PhyphoxBLE::disconnected(){
  #ifdef DEBUG
  if(printer != nullptr){
    printer -> println("device disconnected");
  }
  #endif
  myAdvertising->start();
}


void PhyphoxBLE::begin(HardwareSerial* hwPrint)
{
  #ifdef DEBUG
	printer = hwPrint;
  if(printer)
	   printer->begin(115200);
  #endif
}

void PhyphoxBLE::printXML(HardwareSerial* printer){
  printer->println("");
  for(int i =0; i<expLen;i++){
      char CHAR = EXPARRAY[i];
      printer->print(CHAR);
  }
  printer->println("");
}

#endif
