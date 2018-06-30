#!/usr/bin/python
import os
import sqlite3
import time
import signal
from meter import Meter
from Init import FrequencyScan
from MQTT import MQTT
import threading
import requests
import RPi.GPIO as GPIO
import pigpio
import sys, traceback


def main():
    print('\nStarting SMI - Core...\n')
    os.system('systemctl stop serial-getty@ttyAMA0.service')
    time.sleep(3)
    print('\nLoading Meters...\n')
    loadPower = 1
    Cloud = MQTT()
    Scan = FrequencyScan()
    Frequency = AdjustParam()
    Meters = []
    Meters = DownloadMeters(Frequency)
    adjustParam = 0
    init = 1
    Flag = 1


    try:
        print('\nStarting Meters...\n')
        MeterIdx = []
        MeterIdx = StartMeters(Meters)
        while True:
            #time.sleep(15)
            metering = []
            relayValue = [0] *10
            relayIntermedValue = []
            relayInfo = []
            del relayInfo[:]
            #del relayValue[:]
            metering_lastid = 0
            iThreshold = 0.1
            iCount = 0

            for MeterIdx in Meters:
                MQTTCommand(MeterIdx, iThreshold, Flag)
                if (MeterIdx[4].meteringAvailable() == True):
                    url = "http://delso.pythonanywhere.com/SelecionarFlags%i" % (MeterIdx[0])
                    Flag = UrlResponseVerification(url)

                    metering = MeterIdx[4].meteringGet()

                    for metering_row in metering:
                    # Store metering in the database
                    # Create metering record
                        url = "http://delso.pythonanywhere.com/InsereMeterFlagNovaMedicao%iFlag%i" % (
                        MeterIdx[0], Flag)
                        resp = UrlResponseVerification(url)

                    # Get last Metering Id
                    url = "http://delso.pythonanywhere.com/LastMeteringIdRasp"
                    metering_lastid = UrlResponseVerification(url)

                    #     # Insert Data code 1 = voltage (V)
                    #     # Data code 2 = current (A)
                    #     # Data code 3 = power factor
                    #     # Data code 4 = tot wh (wh)
                    #     # Data code 5 = tot varh (varh)
                    #     # Data code 6 = Pot (W)


                    url = "http://delso.pythonanywhere.com/InserirDados/ID%i_METERINGMETERID%i__DATA1_VALUE%f__DATA2_VALUE%f__DATA3_VALUE%f__DATA4_VALUE%f__DATA5_VALUE%f" \
                          % (metering_lastid, MeterIdx[0], metering_row[1], metering_row[2], metering_row[3],
                             metering_row[4], metering_row[5])
                    resp = UrlResponseVerification(url)
					
                    VoltageMQTT = ("/Medidor/%i/Tensao") %(MeterIdx[0])
                    CorrenteMQTT = ("/Medidor/%i/Corrente") % (MeterIdx[0])
                    FPMQTT = ("/Medidor/%i/FP") % (MeterIdx[0])
                    EnergiaMQTT = ("/Medidor/%i/Energia") % (MeterIdx[0])
                    ReativaMQTT = ("/Medidor/%i/Reativa") % (MeterIdx[0])
                    #
                    Cloud.client.publish(VoltageMQTT, ("Tensão = %fV" %(metering_row[1])))
                    Cloud.client.publish(CorrenteMQTT, ("Corrente = %fA" % (metering_row[2])))
                    Cloud.client.publish(FPMQTT, ("Fator de potencia = %f" % (metering_row[3])))
                    Cloud.client.publish(EnergiaMQTT, ("Energia = %fWh" % (metering_row[4])))
                    Cloud.client.publish(ReativaMQTT, ("Energia Reativa = %fVArh" % (metering_row[5])))
                    metering = []


    except KeyboardInterrupt:
        print('Stopping SMI...\n')
        Scan.pi.set_mode(15, pigpio.ALT0)  # GPIO 15 as RX
        traceback.print_exc(file=sys.stdout)
        for MeterIdx in Meters:
            GPIO.cleanup()
            print('Stopping [%s]' % (MeterIdx[1]))
            MeterIdx[4].TurnOffPWM()
            if MeterIdx[4].isAlive(): MeterIdx[4].kill_received = True
            pid = os.getpid()
            os.kill(pid, signal.SIGKILL)

    except Exception:
        print('Stopping SMI...\n')
        Scan.pi.set_mode(15, pigpio.ALT0)  # GPIO 15 as RX
        traceback.print_exc(file=sys.stdout)
        for MeterIdx in Meters:
            GPIO.cleanup()
            print('Stopping [%s]' % (MeterIdx[1]))
            MeterIdx[4].TurnOffPWM()
            if MeterIdx[4].isAlive(): MeterIdx[4].kill_received = True
            pid = os.getpid()
            os.kill(pid, signal.SIGKILL)

def UrlResponseVerification(url):
    response = requests.get(url)
    while (response.status_code != 200):
        if (response.status_code == 500):
            print('Erro no acesso ao banco')
            print('nova conexão em 1 segundos...')
            time.sleep(0.1)
            response = requests.get(url)
        elif (response.status_code == 404):
            print('Erro no acesso ao banco')
            print('nova conexão em 6 segundos...')
            time.sleep(1)
            response = requests.get(url)
    print('Acesso ao banco realizado com sucesso')
    return response.json()

def AdjustParam():
    Scan = FrequencyScan()
    Gain = Scan.GainAdjust()
    time.sleep(10)
    Frequency = Scan.FrequencyAdjust()
    return Frequency

def DownloadMeters(Frequency):
    Meters = []
    url = "http://delso.pythonanywhere.com/Meter"
    resp = UrlResponseVerification(url)
    print(resp)  ##INSERT
    for Row in resp:
        Meters = Meters + [[Row[0], Row[1], Row[2], Row[3], Meter(Row[2], Frequency)]]
    print('Meters %s\n' % Meters)
    return Meters

def StartMeters(Meters):
    for MeterIdx in Meters:
        print('[%s] Init' % (MeterIdx[1]))  
        MeterIdx[4].start()
        time.sleep(1)
    return MeterIdx

## 5 Meters Control
def MQTTCommand(MeterId,iThreshold,MeterFlag):
    if MeterId[0] == 1:
        if MeterFlag == 0:
            if MQTT.RelayCommandId_1 == 1:
                if  MeterId[4].meteringON(MeterId[0]):
                    print('Carga do Medidor %i ligada' % (MeterId[0]))
                    url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=1_Meter=%i" % (MeterId[0])
                    teste1 = UrlResponseVerification(url)

        elif MeterFlag == 1:
            if MQTT.SBCommandId_1 == 1:
                if MQTT.RelayCommandId_1 == 0:
                # if FlagLigado == 0:
                    url = "http://delso.pythonanywhere.com/LastestMetering/Medidor%i/%f" % (MeterId[0], iThreshold)
                    teste1 = UrlResponseVerification(url)
                    if teste1 == "D":
                        if MeterId[4].meteringOFF(MeterId[0]):
                            print('Carga do Medidor %i Desligado StandBy' % MeterId[0])
                            url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                            teste1 = UrlResponseVerification(url)
                        else:
                            print("Erro ao desligar a carga %i" %(MeterId[0]))

            else:
                if MQTT.RelayCommandId_1 == 0:
                # if FlagLigado == 0:
                    if MeterId[4].meteringOFF(MeterId[0]):  # Função de desligamento
                        print('Carga do Medidor %i desligado' % (MeterId[0]))
                        url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                        teste1 = UrlResponseVerification(url)
                    else:
                        print("Erro ao desligar a carga %i" % (MeterId[0]))

    elif MeterId[0] == 2:
        if MeterFlag == 0:
            if MQTT.RelayCommandId_2 == 1:
                if  MeterId[4].meteringON(MeterId[0]):
                    print('Carga do Medidor %i ligada' % (MeterId[0]))
                    url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=1_Meter=%i" % (MeterId[0])
                    teste1 = UrlResponseVerification(url)

        elif MeterFlag == 1:
            if MQTT.SBCommandId_2 == 1:
                if MQTT.RelayCommandId_2 == 0:
                # if FlagLigado == 0:
                    url = "http://delso.pythonanywhere.com/LastestMetering/Medidor%i/%f" % (MeterId[0], iThreshold)
                    teste1 = UrlResponseVerification(url)
                    if teste1 == "D":
                        if MeterId[4].meteringOFF(MeterId[0]):
                            print('Carga do Medidor %i Desligado StandBy' % MeterId[0])
                            url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                            teste1 = UrlResponseVerification(url)
                        else:
                            print("Erro ao desligar a carga %i" %(MeterId[0]))

            else:
                if MQTT.RelayCommandId_2 == 0:
                # if FlagLigado == 0:
                    if MeterId[4].meteringOFF(MeterId[0]):  # Função de desligamento
                        print('Carga do Medidor %i desligado' % (MeterId[0]))
                        url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                        teste1 = UrlResponseVerification(url)
                    else:
                        print("Erro ao desligar a carga %i" % (MeterId[0]))

    elif MeterId[0] == 3:
        if MeterFlag == 0:
            if MQTT.RelayCommandId_3 == 1:
                if  MeterId[4].meteringON(MeterId[0]):
                    print('Carga do Medidor %i ligada' % (MeterId[0]))
                    url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=1_Meter=%i" % (MeterId[0])
                    teste1 = UrlResponseVerification(url)

        elif MeterFlag == 1:
            if MQTT.SBCommandId_3 == 1:
                if MQTT.RelayCommandId_3 == 0:
                # if FlagLigado == 0:
                    url = "http://delso.pythonanywhere.com/LastestMetering/Medidor%i/%f" % (MeterId[0], iThreshold)
                    teste1 = UrlResponseVerification(url)
                    if teste1 == "D":
                        if MeterId[4].meteringOFF(MeterId[0]):
                            print('Carga do Medidor %i Desligado StandBy' % MeterId[0])
                            url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                            teste1 = UrlResponseVerification(url)
                        else:
                            print("Erro ao desligar a carga %i" %(MeterId[0]))

            else:
                if MQTT.RelayCommandId_3 == 0:
                # if FlagLigado == 0:
                    if MeterId[4].meteringOFF(MeterId[0]):  # Função de desligamento
                        print('Carga do Medidor %i desligado' % (MeterId[0]))
                        url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                        teste1 = UrlResponseVerification(url)
                    else:
                        print("Erro ao desligar a carga %i" % (MeterId[0]))

    elif MeterId[0] == 4:
        if MeterFlag == 0:
            if MQTT.RelayCommandId_4 == 1:
                if  MeterId[4].meteringON(MeterId[0]):
                    print('Carga do Medidor %i ligada' % (MeterId[0]))
                    url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=1_Meter=%i" % (MeterId[0])
                    teste1 = UrlResponseVerification(url)

        elif MeterFlag == 1:
            if MQTT.SBCommandId_4 == 1:
                if MQTT.RelayCommandId_4 == 0:
                # if FlagLigado == 0:
                    url = "http://delso.pythonanywhere.com/LastestMetering/Medidor%i/%f" % (MeterId[0], iThreshold)
                    teste1 = UrlResponseVerification(url)
                    if teste1 == "D":
                        if MeterId[4].meteringOFF(MeterId[0]):
                            print('Carga do Medidor %i Desligado StandBy' % MeterId[0])
                            url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                            teste1 = UrlResponseVerification(url)
                        else:
                            print("Erro ao desligar a carga %i" %(MeterId[0]))

            else:
                if MQTT.RelayCommandId_4 == 0:
                # if FlagLigado == 0:
                    if MeterId[4].meteringOFF(MeterId[0]):  # Função de desligamento
                        print('Carga do Medidor %i desligado' % (MeterId[0]))
                        url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                        teste1 = UrlResponseVerification(url)
                    else:
                        print("Erro ao desligar a carga %i" % (MeterId[0]))

    elif MeterId[0] == 5:
        if MeterFlag == 0:
            if MQTT.RelayCommandId_5 == 1:
                if  MeterId[4].meteringON(MeterId[0]):
                    print('Carga do Medidor %i ligada' % (MeterId[0]))
                    url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=1_Meter=%i" % (MeterId[0])
                    teste1 = UrlResponseVerification(url)

        elif MeterFlag == 1:
            if MQTT.SBCommandId_5 == 1:
                if MQTT.RelayCommandId_5 == 0:
                # if FlagLigado == 0:
                    url = "http://delso.pythonanywhere.com/LastestMetering/Medidor%i/%f" % (MeterId[0], iThreshold)
                    teste1 = UrlResponseVerification(url)
                    if teste1 == "D":
                        if MeterId[4].meteringOFF(MeterId[0]):
                            print('Carga do Medidor %i Desligado StandBy' % MeterId[0])
                            url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                            teste1 = UrlResponseVerification(url)
                        else:
                            print("Erro ao desligar a carga %i" %(MeterId[0]))

            else:
                if MQTT.RelayCommandId_5 == 0:
                # if FlagLigado == 0:
                    if MeterId[4].meteringOFF(MeterId[0]):  # Função de desligamento
                        print('Carga do Medidor %i desligado' % (MeterId[0]))
                        url = "http://delso.pythonanywhere.com/AtualizaMeterFlag=0_Meter=%i" % (MeterId[0])
                        teste1 = UrlResponseVerification(url)
                    else:
                        print("Erro ao desligar a carga %i" % (MeterId[0]))

						
if __name__ == "__main__":
    main()