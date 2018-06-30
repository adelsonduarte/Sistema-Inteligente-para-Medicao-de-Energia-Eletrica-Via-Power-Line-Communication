import paho.mqtt.client as mqtt, os, urllib.parse
import time


# Define event callbacks

class MQTT():
    RelayCommandId_1 = 1
    RelayCommandId_2 = 1
    RelayCommandId_3 = 1
    RelayCommandId_4 = 1
    RelayCommandId_5 = 1
    SBCommandId_1 = -1
    SBCommandId_2 = -1
    SBCommandId_3 = -1
    SBCommandId_4 = -1
    SBCommandId_5 = -1
    def TrataComando(topic, command):
        _topic = topic
        _command = command

        if _topic == "/Medidor/1/Comando":
            if _command == "1":
                MQTT.RelayCommandId_1 = 1
            else:
                MQTT.RelayCommandId_1 = 0

        elif _topic == "/Medidor/1/Standby":
            if _command == "1":
                MQTT.SBCommandId_1 = 1
            else:
                MQTT.SBCommandId_1 = 0

        elif _topic == "/Medidor/2/Comando":
            if _command == "1":
                MQTT.RelayCommandId_2 = 1
            else:
                MQTT.RelayCommandId_2 = 0

        elif _topic == "/Medidor/2/Standby":
            if _command == "1":
                MQTT.SBCommandId_2 = 1
            else:
                MQTT.SBCommandId_2 = 0

        elif _topic == "/Medidor/3/Comando":
            if _command == "1":
                MQTT.RelayCommandId_3 = 1
            else:
                MQTT.RelayCommandId_3 = 0

        elif _topic == "/Medidor/3/Standby":
            if _command == "1":
                MQTT.SBCommandId_3 = 1
            else:
                MQTT.SBCommandId_3 = 0

        elif _topic == "/Medidor/4/Comando":
            if _command == "1":
                MQTT.RelayCommandId_4 = 1
            else:
                MQTT.RelayCommandId_4 = 0

        elif _topic == "/Medidor/4/Standby":
            if _command == "1":
                MQTT.SBCommandId_4 = 1
            else:
                MQTT.SBCommandId_4 = 0

        elif _topic == "/Medidor/5/Comando":
            if _command == "1":
                MQTT.RelayCommandId_5 = 1
            else:
                MQTT.RelayCommandId_5 = 0

        elif _topic == "/Medidor/5/Standby":
            if _command == "1":
                MQTT.SBCommandId_5 = 1
            else:
                MQTT.SBCommandId_5 = 0

    def on_connect(mosq, obj, rc):
        # print ("on_connect:: Connected with result code "+ str ( rc ) )
        # print("rc: " + str(rc))
        print("")

    def on_message(mosq, obj, msg):
        print ("on_message:: this means  I got a message from brokerfor this topic")
        Teste = msg.topic
        Teste2 = str(int(msg.payload))
        print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
        MQTT.TrataComando(Teste, Teste2)
        print("")


    def on_publish(mosq, obj, mid):
        # print("mid: " + str(mid))
        print("")


    def on_subscribe(mosq, obj, mid, granted_qos):
        print("This means broker has acknowledged my subscribe request")
        # print("Subscribed: " + str(mid) + " " + str(granted_qos))


    def on_log(mosq, obj, level, string):
        print(string)

    # if init == True:
    client = mqtt.Client()
    # Assign event callbacks
    client.on_message = on_message
    client.on_connect = on_connect
    client.on_publish = on_publish
    client.on_subscribe = on_subscribe

    # Uncomment to enable debug messages
    client.on_log = on_log

    # user name has to be called before connect - my notes.
    client.username_pw_set("tdqtilqg", "DA-c7ZBF0YQA")

    client.connect('m10.cloudmqtt.com', 15435, 60)

    # Continue the network loop, exit when an error occurs
    # rc = 0
    # while rc == 0:
    #    rc = client.loop()
    # print("rc: " + str(rc))

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    # client.loop_forever()
    # client.subscribe("/teste", 0)
    #client.subscribe("/Medidor/1/Comando", 0)
    client.subscribe([("/Medidor/1/Comando", 0), ("/Medidor/1/Standby", 0),
                      ("/Medidor/2/Comando", 0), ("/Medidor/2/Standby", 0),
                      ("/Medidor/3/Comando", 0), ("/Medidor/3/Standby", 0),
                      ("/Medidor/4/Comando", 0), ("/Medidor/4/Standby", 0),
                      ("/Medidor/5/Comando", 0), ("/Medidor/5/Standby", 0)])

    client.loop_start()
