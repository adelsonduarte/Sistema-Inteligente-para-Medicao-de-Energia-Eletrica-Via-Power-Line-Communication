import time
import serial
import os
from threading import Thread, Lock
import threading
import struct
from time import gmtime, strftime
from Init import FrequencyScan
import binascii
import pigpio
import RPi.GPIO as GPIO


class Meter(Thread):
    _address = None
    _addressControl = 0
    _SerialConfig = FrequencyScan()
    _metering = []  # timestamp, voltage, current, pf, wh, varh
    _meteringReady = False
    _soframe = b'*'
    _eoframe = '0x21'
    _lock = threading.Lock()
    _Frequency = 0
    _AdjustParams = 0

    def __init__(self, address=None, Frequency = 0):

        Thread.__init__(self)
        self._address = address
        self._Frequency = Frequency
        self._stop_event = threading.Event()

    def stop(self):
        self._stop_event.set()


    def run(self):
        Scan = FrequencyScan()
        print ('## Starting Meter at Address ' + str(self._address))
        SerialControl = self._SerialConfig._commport
        SerialControl.flushInput()
        SerialControl.flushOutput()
        self.pi = pigpio.pi()
        self.pi.set_mode(18, pigpio.OUTPUT)

        Frequencies = FrequencyScan._Frequency
        OKcount = 0
        Errorcount = 0
        total = 0

        while True:
            print('Medidor '+ str(self._address) + ' aguardando...' )
            with self._lock:
                SerialControl.flushInput()
                SerialControl.flushOutput()
                GPIO.output(5, GPIO.HIGH)
                #demora +- 50ms
                rxdata = []
                rxeco = []
                txdata = []
                voltage = 0.0
                current = 0.0
                wh = 0.0
                varh = 0.0
                fp = 0.0


                print('## Reading Meter ' + str(self._address))

                # Try to send the command to first meter
                # monta a mensagem
                send = ("#" + str(self._address) + "g" + ";")
                send_byte = send.encode('ascii')

                # Envio e recebimento
                self.pi.hardware_PWM(18, Frequencies[self._Frequency] , 500000)  # 1.4MHz, 50%
                # txdata = self._commport.write(send_byte)
                txdata = SerialControl.write(send_byte)
                rxeco = SerialControl.read(4)  # eco

                rxdata = SerialControl.read(1)
                #Verifica o tamanho da resposta
                if len(rxdata):
                    if rxdata == self._soframe: #Verifica o header de inicio
                        # rxdata = rxdata + self._commport.read(13)
                        rxdata = rxdata + SerialControl.read(13)

                        if len(rxdata) == 14:
                            hexarxdata =  hex(rxdata[13])
							
                            # End of Frame check
                            if hexarxdata == self._eoframe: #Verifica o header de fim

                                #Lê os paramêtros enviados
                                timestamp = strftime("%Y-%m-%d %H:%M:%S", gmtime())
                                voltage, = struct.unpack('>f', rxdata[1:5])
                                current, = struct.unpack('>f', rxdata[5:9])
                                wh, = struct.unpack('>f', rxdata[9:13])
                                print("## Voltage %f Current %f Wh %f \n " % (voltage, current, wh))
                                self._metering = self._metering + [[timestamp, voltage, current, 0.0, wh, 0.0]]
                                self._meteringReady = True
                                print('## Transmissão Completa')

                            else:
                                print('## Invalid End of Frame Meter ' + str(self._address) + '\n\n')
                                self._AdjustParams = self._AdjustParams + 1
                        else:
                            print('## Frame Size Error Meter ' + str(self._address) + '\n\n')
                            self._AdjustParams = self._AdjustParams + 1

                    else:
                        print('## Invalid Start of Frame Meter ' + str(self._address) + '\n\n')
                        self._AdjustParams = self._AdjustParams + 1

                else:
                    print('## Meter ' + str(self._address) + ' RX < TIMEOUT\n')

                self.pi.hardware_PWM(18, 0, 0)  # 1.4MHz, 50%
                GPIO.output(5, GPIO.LOW)
                time.sleep(5)

    def AdjustParamCount(self,C):
        if C == 1:
            return self._AdjustParams
        elif C == 0:
            self._AdjustParams = 0

    def TurnOffPWM(self):
        self.pi.hardware_PWM(18, 0, 0)  # 50%

    def meteringAvailable(self):

        return self._meteringReady

    def meteringGet(self):
        # self._lock.acquire()
        meteringNow = []
        meteringNow = self._metering
        self._meteringReady = False
        self._metering = []
        # self._lock.release()
        return meteringNow

    #Função para desligar medidor
    def meteringOFF(self, addressOFF):
        # self._lock.acquire()
        SerialControl = self._SerialConfig._commport
        Frequencies = FrequencyScan._Frequency
        self._addressControl = addressOFF
        rxdata = []
        txdata = []
        # send comando para desligar medidor
        self.pi.hardware_PWM(18, Frequencies[self._Frequency], 500000)  # 1.4MHz, 50%
        send1 = ("#" + str(self._addressControl) + "r" + ";")
        # txdata = self._commport.write(send1.encode('ascii'))
        # rxeco = self._commport.read(4)
        # rxdata = self._commport.read(1)
        txdata = SerialControl.write(send1.encode('ascii'))
        rxeco = SerialControl.read(4)
        self.pi.hardware_PWM(18, 0, 0)  # 1.4MHz, 50%
        return 1
        #Resposta RX = *1o; (*,Medidor 1, OFF,!)

    #Função para Ligar medidor
    def meteringON(self, addressON):
        SerialControl = self._SerialConfig._commport
        Frequencies = FrequencyScan._Frequency
        self._addressControl = addressON
        rxdata = []
        txdata = []
        # send comando para desligar medidor
        self.pi.hardware_PWM(18, Frequencies[self._Frequency], 500000)  # 1.4MHz, 50%
        send = ("#" + str(self._addressControl) + "s" + ";")
        txdata = SerialControl.write(send.encode('ascii'))
        rxeco = SerialControl.read(4)
        self.pi.hardware_PWM(18, 0, 0)  # 1.4MHz, 50%
        return 1
        # Resposta RX = *1i; (*,Medidor 1, ON,!)
