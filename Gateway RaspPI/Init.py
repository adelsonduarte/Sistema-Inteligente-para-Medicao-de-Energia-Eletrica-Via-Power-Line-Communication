#!/usr/bin/python

import os
import time
import pigpio
import serial
from operator import xor
import RPi.GPIO as GPIO
import sys, traceback

os.system('pigpiod')

class FrequencyScan():
    _commport = '/dev/ttyAMA0'
    _testframe = b'T'
    _Frequency = [505000, 517000, 530000, 543000, 558000, 573000, 589000,
                  606000, 624000, 643000, 663000, 684000, 707000, 731000,
                  757500, 785500, 815769, 848400, 883750, 992173, 964090,
                  1010000, 1060500]
    _FrequencyAmount = 23
    _start = 1
    _FreqIdx = 0
    _PortFlag = 1
    pi = pigpio.pi()


    def __init__(self):
        self._commport = serial.Serial(self._commport, baudrate=9600, timeout=2, xonxoff=False, rtscts=False,
                                       dsrdtr=False)
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(6, GPIO.OUT)
        GPIO.setup(26, GPIO.OUT)
        GPIO.setup(19, GPIO.OUT)
        GPIO.setup(5, GPIO.OUT)
        GPIO.output(6, GPIO.LOW)
        GPIO.output(26, GPIO.LOW)
        GPIO.output(19, GPIO.LOW)
        GPIO.output(5, GPIO.LOW)

    def FrequencyAdjust(self):
        self.pi.set_mode(18, pigpio.OUTPUT)
        kcounter = 0
        k = [0]*23
        s = [0]*23
        # k = []
        # s = []
        Frequency = 0
        _FreqIdx = self._FreqIdx
        _start = self._start
        print('Inicio da varredura de frequência')
        self._commport.flushInput()
        self._commport.flushOutput()
        for contador in range(self._FrequencyAmount):
            send = ("T")
            send_byte = send.encode('ascii')
            self.pi.hardware_PWM(18, self._Frequency[contador], 500000)  # 1.4MHz, 50%
            txdata = self._commport.write(send_byte)
            self.pi.hardware_PWM(18, 0, 500000)  # 1.4MHz, 50%
            time.sleep(0.1)
            rxeco = self._commport.read(1)  # eco
            if (rxeco == self._testframe):
                print('Frequência %i OK' % (self._Frequency[contador]))
                k[contador] = 1
                kcounter = kcounter + 1
                s[contador] = contador
            else:
                print('Erro na Frequência %i' % (self._Frequency[contador]))
                k[contador] = 0
                s[contador] = 0

            _FreqIdx += s[contador]
            # time.sleep(10)

        FrequencyIdx = int(_FreqIdx/kcounter)
        print('Média = %i' % (FrequencyIdx))
        print('Frequência selecionada = %iHz' % self._Frequency[FrequencyIdx])
        return FrequencyIdx

    def GainAdjust(self):
        # GPIO26    GPIO6
        # Ganho 2   Ganho 1     R               Ganho               R = 470
        # 0         0           470              2.6                R1 = 470
        # 0         1           (R//R1)          4.2                R2 = 150
        # 1         0           (R//R2)          6.0
        # 1         1           (R//R1//R2)      7.6

        c = 0
        GPIO.setup(15, GPIO.IN)
        GPIO.add_event_detect(15, GPIO.FALLING, callback=self.RXinterrupt)
        State = 1
        while (c != 1):
            if (State == 1):
                GPIO.output(6, GPIO.LOW)
                GPIO.output(26, GPIO.LOW)
                time.sleep(1)
                if (self._PortFlag == 1):
                    State = 2
                else:
                    c = 1

            elif (State == 2):
                GPIO.output(6, GPIO.HIGH)
                GPIO.output(26, GPIO.LOW)
                time.sleep(1)
                if (self._PortFlag == 1):
                    State = 3
                else:
                    GPIO.output(6, GPIO.LOW)
                    GPIO.output(26, GPIO.LOW)
                    State = 1
                    c = 1

            elif (State == 3):
                GPIO.output(6, GPIO.LOW)
                GPIO.output(26, GPIO.HIGH)
                time.sleep(1)
                if (self._PortFlag == 1):
                    State = 4
                else:
                    GPIO.output(6, GPIO.HIGH)
                    GPIO.output(26, GPIO.LOW)
                    State = 2
                    c = 1

            elif (State == 4):
                GPIO.output(6, GPIO.HIGH)
                time.sleep(1)
                if (self._PortFlag == 1):
                    c = 1
                else:
                    GPIO.output(6, GPIO.LOW)
                    GPIO.output(26, GPIO.HIGH)
                    State = 3
                    c = 1

        print('Ganho ajustado = %i' % (State))
        GPIO.remove_event_detect(15)
        self.pi.set_mode(15, pigpio.ALT0)  # GPIO 15 as RX
        return (State)

    def RXinterrupt(self, channel):
        _PortFlag = self._PortFlag
        print('Interrupção em RX')
        _PortFlag = 0
        self._PortFlag = _PortFlag


















