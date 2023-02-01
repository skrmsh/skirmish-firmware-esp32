/*
Skirmish ESP32 Firmware

Bluetooth Low Energy driver

Copyright (C) 2023 Ole Lange
*/

#include <inc/bluetooth.h>
#include <inc/log.h>
#include <inc/const.h>
#include <conf.h>

#include <ArduinoJson.h>

// Prefixes for the BLE name
#if MODULE_TYPE == MODULE_PHASER
    const char* namePrefix = "skrmphsr";
#elif MODULE_TYPE == MODULE_CHEST
    const char* namePrefix = "skrmvest";
#elif MODULE_TYPE == MODULE_HITPOINT
    const char* namePrefix = "skrmhtpt";
#endif


// BLE Service and Characteristic UUIDs
#define SERVICE_UUID              "b9f96468-246b-4cad-a3e2-e4c282280852" // skirmish service
#define WRITE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" // json write chrst
#define READ_CHARACTERISTIC_UUID  "beb5483f-36e1-4688-b7f5-ea07361b26a8" // json read chrst

/**
 * Callback Handler class for the Server
 * containing callbacks for connect and disconnect events
*/
class ServerCallbacks : public BLEServerCallbacks {

    private:
    SkirmishBluetooth *ble; // Containing the bluetooth driver

    public:
    /**
     * Updates/Sets the currently used bluetooth driver
     * 
     * @param ble bluetooth driver
    */
    void setBluetoothDriver(SkirmishBluetooth *ble) {
        this->ble = ble;
    }
    
    /**
     * Is called when the ble server is connected
    */
    void onConnect(BLEServer *server) {
        logInfo("Bluetooth connected!");
        ble->setConnectionState(true);
        ble->onConnectCallback(this->ble->com);
    }

    /**
     * Is called when the ble server was disconnected
    */
    void onDisconnect(BLEServer *server) {
        logError("Bluetooth disconnected!");
        ble->setConnectionState(false);
        ble->onDisconnectCallback(this->ble->com);
        ble->startAdvertising();
    }
};

/**
 * Callback Handler class for notified characteristicsc
 * contains callback for newly received data
*/
class CharacteristcCallbacks : public BLECharacteristicCallbacks {
    private:
    SkirmishBluetooth *ble; // Containing the bluetooth driver
    DynamicJsonDocument *parsedJsonData = new DynamicJsonDocument(1024);

    public:
    /**
     * Updates/Sets the currently used bluetooth driver
     * 
     * @param ble bluetooth driver
    */
    void setBluetoothDriver(SkirmishBluetooth *ble) {
        this->ble = ble;
    }

    /**
     * Is called when the characteristic is notified about new data
    */
    void onWrite(BLECharacteristic *characteristic) {
        parsedJsonData->clear();
        deserializeJson(*parsedJsonData, characteristic->getValue());
        this->ble->onReceiveCallback(this->ble->com, parsedJsonData);
    }
};

/**
 * Constructor.
*/
SkirmishBluetooth::SkirmishBluetooth() {
}

/**
 * Sets the SkirmCom instance
*/
void SkirmishBluetooth::setCom(void *com) {
    this->com = com;
}

/**
 * Initializes the bluetooth le driver
*/
void SkirmishBluetooth::init() {
    logInfo("Init: Bluetooth driver");

    // Generate the chip ID based on the MAC addr
    uint32_t chipID = 0;
    for (uint8_t i = 0; i < 17; i += 8) {
        chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    logDebug("-> My Chip ID is: %08x", chipID);

    // Generating bluetoothName using chipID and depending on the module
    // the correct prefix. The 17 used in the malloc function is the length
    // of the prefix (8) + the length of the chipID in hex (8) + termination (1)
    bluetoothName = (char*) malloc(17 * sizeof(char));
    sprintf(bluetoothName, "%s%08x", namePrefix, chipID);

    logInfo("-> My bluetooth name is %s", bluetoothName);

    // Initing BLE device and create service and characterics
    BLEDevice::init(bluetoothName);

    server = BLEDevice::createServer();
    BLEService *service = server->createService(SERVICE_UUID);

    
    // The read and write characteristics of the connected smartphone/device
    // allow for communication between the two devices. It is similar to UART
    // over BLE, but implemented manually. To keep the solution scalable, a
    // JSON object is sent directly from the server to the device and interpreted
    // on the device, rather than using a separate characteristic for each field
    // or attribute.

    writeCharacteristic = service->createCharacteristic(
                            WRITE_CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_WRITE |
                            BLECharacteristic::PROPERTY_READ
                        );
    readCharacteristic = service->createCharacteristic(
                            READ_CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_READ |
                            BLECharacteristic::PROPERTY_NOTIFY |
                            BLECharacteristic::PROPERTY_INDICATE
                        );

    // Required for IOS to prevent error on subscription
    BLEDescriptor notifyDescriptor = 
        BLEDescriptor("00002902-0000-1000-8000-00805f9b34fb");
    readCharacteristic->addDescriptor(&notifyDescriptor);

    // Start advertising the available service
    BLEAdvertising *advertising = server->getAdvertising();
    advertising->start();

    // Set callbacks (for connect/disconnect and notification about new data)
    ServerCallbacks *scb = new ServerCallbacks();
    CharacteristcCallbacks *ccb = new CharacteristcCallbacks();
    scb->setBluetoothDriver(this);
    ccb->setBluetoothDriver(this);
    server->setCallbacks(scb);
    writeCharacteristic->setCallbacks(ccb);
    logDebug("-> Set server/characteristics callbacks");

    // Start the service
    service->start();
}

/**
 * Starts advertising the ble server. Must be called to allow reconnect after
 * a disconnect
*/
void SkirmishBluetooth::startAdvertising() {
    server->startAdvertising();
}

/**
 * @return True if the bluetooth driver is currently connected
*/
bool SkirmishBluetooth::getConnectionState() {
    return isConnected;
}

/**
 * Sets the current connection state. Should only be called from ble driver
 * callbacks (scb/ccb)
 * 
 * @param newState new connection state
*/
void SkirmishBluetooth::setConnectionState(bool newState) {
    isConnected = newState;
}

/**
 * @return the bluetooth name string
*/
char* SkirmishBluetooth::getName() {
    return bluetoothName;
}

/**
 * Writes data to the readCharacteristic an notifies the deivce on
 * the other end.
 * 
 * @param data the json data which should be send to client
*/
void SkirmishBluetooth::writeJsonData(DynamicJsonDocument *data) {
    serializeJson(*data, dataBuffer);
    readCharacteristic->setValue(dataBuffer);
    readCharacteristic->notify();
}

/**
 * Sets the callback that gets called on newly received data
 * 
 * @param callback the callback function taking one parameter of type DynamicJsonDocument *
*/
void SkirmishBluetooth::setOnReceiveCallback(void(* callback)(void *context, DynamicJsonDocument*)) {
    this->onReceiveCallback = callback;
}

/**
 * Sets the callback that gets called on connection
 * 
 * @param callback the callback function
*/
void SkirmishBluetooth::setOnConnectCallback(void(*callback)(void *context)) {
    this->onConnectCallback = callback;
}

/**
 * Sets the callback that gets called when the device was disconnected
 * 
 * @param callback the callback function
*/
void SkirmishBluetooth::setOnDisconnectCallback(void(*callback)(void *context)) {
    this->onDisconnectCallback = callback;
}