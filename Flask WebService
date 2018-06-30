from flask import Flask, render_template
from flask.ext.sqlalchemy import SQLAlchemy
from sqlalchemy import create_engine
from time import gmtime, strftime, strptime
from datetime import datetime
import MySQLdb
from flask_restful import Resource, Api
import pytz

app = Flask(__name__)
api = Api(app)

db_connect = create_engine('mysql://****:***@delso.mysql.pythonanywhere-services.com:3306/delso$RemoteMetermysql')

                ##Start of creation of dynamic web pages ##
## Web Homepage
@app.route('/')
def index():
    return render_template('HomepageV4.html')


## Page of Meter parameters
@app.route('/Medidor<int:MedidorId>')
def Medidor_index(MedidorId):
    MeteringData = []
    LastMeteringId = []
    MeterId = MedidorId
    try:
        conn = db_connect.connect()
        query = conn.execute("Select Max(Id) from Metering where MeterId = %i" % (MedidorId))
        for i in query.cursor.fetchall():
            LastMeteringId = i[0]
        if LastMeteringId == None:
            return render_template('ErrorpageV4.html')
        query = conn.execute("Select TimeStamp from Metering where Id = %i" % (LastMeteringId))
        for i in query.cursor.fetchall():
            LastTimeStamp = i[0]

        _formatedTime = FormatTime(LastTimeStamp)
        query = conn.execute("select Value from MeteringData where MeteringId = %i " % LastMeteringId)
        for i in query.cursor.fetchall():
            MeteringData = MeteringData + [i[0]]
        if MeterId == 4:
            return render_template('MedidorGeralV4.html', Voltage1 = MeteringData[0], Current1 = MeteringData[1],
                               ActiveEnergy = (MeteringData[3] + MeteringData[8] + MeteringData[13]),
                               ReativeEnergy = (MeteringData[4] + MeteringData[9] + MeteringData[14]),
                               Voltage2=MeteringData[5], Current2=MeteringData[6],
                               Voltage3=MeteringData[10], Current3=MeteringData[11],
                               Time = _formatedTime, MeterId = MeterId )
        else:
            return render_template('MedidoresTemplateV4.html', Voltage=MeteringData[0], Current=MeteringData[1],
                                   Factor=MeteringData[2], ActiveEnergy=MeteringData[3], ReativeEnergy=MeteringData[4],
                                   Time=_formatedTime, MeterId=MeterId)
    finally:
        conn.close()


## data history consult page
@app.route('/Medidor<int:MedidorId>/Historico<int:Info>')
def Info(MedidorId,Info):
    _info = Info
    return render_template('HistoricoV4.html',_infoname = InfoName(_info))

                ## Start 3-Phase ##
@app.route('/Medidor<int:MedidorId>/Historico<int:InfoGeral>Fase<int:Fase>')
def GeralInfo(MedidorId,InfoGeral,Fase):
    _infogeral = InfoGeral
    return render_template('HistoricoV4.html',_infoname = InfoName(_infogeral))

@app.route('/Medidor<int:MedidorId>/Historico<int:InfoGeral>/inicio<string:dataStart>fim<string:dataEnd>')
def GeralConsulta(MedidorId, InfoGeral, dataStart,dataEnd):
    _info = InfoGeral
    MeteringId = []
    values = []
    period = []
    try:
        conn = db_connect.connect()
        query = conn.execute("SELECT Id FROM Metering WHERE MeterId = '%i' AND (TimeStamp BETWEEN '%s' AND '%s')" %
                             (MedidorId, dataStart, dataEnd))
        for i in query.cursor.fetchall():
            MeteringId = MeteringId + [i[0]]

        for z in MeteringId:
            query = conn.execute("SELECT Value FROM MeteringData WHERE MeteringId = '%i' AND Data = '%i' "
                                 "AND MeteringMeterId = '%i'" % (z, InfoGeral, MedidorId))
            for i in query.cursor.fetchall():
                values = values + [i[0]]

            query = conn.execute("SELECT TIMESTAMP FROM Metering WHERE Id = '%i'" % (z))
            for y in query.cursor.fetchall():
                _formatedTime = FormatTime(y[0])
                period = period + [_formatedTime]
        return render_template('ConsultaV4.html', values=values, period=period, _infoname=InfoName(_info))
    finally:
        conn.close()
					## End 3-Phase ##

## result page of history consult
@app.route('/Medidor<int:MedidorId>/Historico<int:Info>/inicio<string:dataStart>fim<string:dataEnd>')
def Consulta(MedidorId, Info, dataStart,dataEnd):
    _info = Info
    MeteringId = []
    values = []
    period = []
    try:
        conn = db_connect.connect()
        query = conn.execute("SELECT Id FROM Metering WHERE MeterId = '%i' AND (TimeStamp BETWEEN '%s' AND '%s')" % (MedidorId, dataStart, dataEnd))
        for i in query.cursor.fetchall():
            MeteringId = MeteringId + [i[0]]

        _tamanhoDados = len(MeteringId)
        for z in MeteringId:
            query = conn.execute("SELECT Value FROM MeteringData WHERE MeteringId = '%i' AND Data = '%i' "
                                 "AND MeteringMeterId = '%i'" % (z, Info, MedidorId))
            for i in query.cursor.fetchall():
                values = values + [i[0]]

            query = conn.execute("SELECT TIMESTAMP FROM Metering WHERE Id = '%i'" % (z))
            for y in query.cursor.fetchall():
                _formatedTime = FormatTime(y[0])
                period = period + [_formatedTime]
        return render_template('ConsultaV4.html', values=values, period=period, _infoname=InfoName(_info), _length = _tamanhoDados)
    finally:
        conn.close()


def InfoName(info):
    _info = info
    _infoname = ''
    if _info == 1:
        _infoname = 'Tensão'
    if _info == 2:
        _infoname = 'Corrente'
    if _info == 3:
        _infoname = 'Fator de Potência'
    if _info == 4:
        _infoname = 'Energia Ativa (Consumo)'
    if _info == 5:
        _infoname = 'Energia Reativa'
    if _info == 11:
        _infoname = 'Tensão Fase 1'
    if _info == 21:
        _infoname = 'Corrente Fase 1'
    #if _info == 31:
    #    _infoname = 'Fator de Potência'
    if _info == 41:
        _infoname = 'Energia Ativa'
    if _info == 51:
        _infoname = 'Energia Reativa'
    if _info == 12:
        _infoname = 'Tensão Fase 2'
    if _info == 22:
        _infoname = 'Corrente Fase 2'
    #if _info == 32:
    #    _infoname = 'Fator de Potência'
    #if _info == 42:
    #    _infoname = 'Energia Ativa (Consumo) Fase 2'
    #if _info == 52:
    #    _infoname = 'Reativa Fase 2'
    if _info == 13:
        _infoname = 'Tensão Fase 3'
    if _info == 23:
        _infoname = 'Corrente Fase 3'
    #if _info == 33:
    #    _infoname = 'Fator de Potência'
    #if _info == 43:
    #    _infoname = 'Energia Ativa (Consumo) Fase 3'
    #if _info == 53:
    #    _infoname = 'Reativa Fase 3'
    return _infoname

def FormatTime(dbTimeFormat):

    local_tz = pytz.timezone('America/Sao_Paulo')
    local_dt = dbTimeFormat.replace(tzinfo=pytz.utc).astimezone(local_tz)
    _formatedTime = local_dt.strftime('%d-%m-%Y %H:%M:%S')
    return _formatedTime

                    ##End of the creation of dynamic web pages ##

					## Start Rest API ##

## DEBUG purpose
class MeterName(Resource):
    def get(self):
        MeterName = []
        try:
            conn = db_connect.connect()
            query = conn.execute("SELECT Name, Address FROM Meter")
            for i in query.cursor.fetchall():
                MeterName = MeterName + [i[0], i[1]]
            return MeterName
        finally:
            conn.close()


## API entrance to Meter table
class Meter(Resource):
    def get(self):
        Meter = []
        try:
            conn = db_connect.connect()
            query = conn.execute("SELECT * FROM Meter")
            for i in query.cursor.fetchall():
                Meter = Meter + [[i[0], i[1], i[2], i[3]]]
            return Meter
        finally:
            conn.close()

## API entrance to metering table
class Metering(Resource):
    def get(self):
        Metering = []
        try:
            conn = db_connect.connect()
            query = conn.execute("select Id, MeterId, TimeStamp, MeterFlag  from Metering;")
            for i in query.cursor.fetchall():
                Metering = Metering + [[i[0], i[1], i[2].strftime('%d-%m-%Y %H:%M:%S'), i[3]]]
            return Metering
        finally:
            conn.close()

## API entrance to MeteringData table
class MeteringData(Resource):
    def get(self, LastId):
        MeteringData = []
        try:
            conn = db_connect.connect()
            query = conn.execute("select MeteringId, Data, Value from MeteringData where MeteringId = %i " % LastId)
            for i in query.cursor.fetchall():
                MeteringData = MeteringData + [[i[0], i[1], i[2]]]
            return MeteringData
        finally:
            conn.close()

## API entrance to change Meter table, column Userflag
class ChangeUserFlag(Resource):
    def get(self, Address):
        try:
            conn = db_connect.connect()
            query = conn.execute("UPDATE Meter SET UserFlag = 1 WHERE Address = %s" % (Address))
            return 'User Flag = 1 para o Endereço'
        finally:
            conn.close()

## DEBUG purpose
class ChangeUserFlagTeste(Resource):
    def get(self, Address):
        try:
            conn = db_connect.connect()
            query = conn.execute("UPDATE Meter SET UserFlag = 0 WHERE Address = %s" % (Address))
            return 'User Flag = 0 para o Endereço'
        finally:
            conn.close()

## API entrance to change Meter table, column MeterFlag
class ChangeMeterFlag1(Resource):
    def get(self, Address):
        try:
            conn = db_connect.connect()
            query = conn.execute("UPDATE Meter SET MeterFlag = 1 WHERE Address = %s" % (Address))
            return 'Meter Flag = 1 para o Endereço'
        finally:
            conn.close()


class ChangeMeterFlag0(Resource):
    def get(self, Address):
        try:
            conn = db_connect.connect()
            query = conn.execute("UPDATE Meter SET MeterFlag = 0 WHERE Address = %s" % (Address))
            return 'Meter Flag = 0 para o Endereço'
        finally:
            conn.close()

## API entrance to change Meter table, column Mode
class NormalMode(Resource):
    def get(self, MeterId):
        try:
            conn = db_connect.connect()
            query = conn.execute("UPDATE Meter SET Mode = 1 WHERE Address = %i" % MeterId)
            return 'Mode = 1 para o Endereço'
        finally:
            conn.close()


class StandByMode(Resource):
    def get(self, MeterId):
        try:
            conn = db_connect.connect()
            query = conn.execute("UPDATE Meter SET Mode = 2 WHERE Address = %i" % MeterId)
            return 'Mode = 2 para o Endereço'
        finally:
            conn.close()

## API entrance to return Meter parameters
class Mode(Resource):
    def get(self, MeterId):
        Meter = []
        try:
            conn = db_connect.connect()
            query = conn.execute("Select Id, Mode from Meter WHERE Id = %i" % MeterId)
            for i in query.cursor.fetchall():
                Meter = Meter + [i[0], i[1]]
            return Meter
        finally:
            conn.close()

## API entrance to return LastMeteringId from Metering table
class LastMeteringId(Resource):
    def get(self, MeterId):
        LastMeteringId = []
        try:
            conn = db_connect.connect()
            query = conn.execute("Select MeterId, Max(Id) from Metering where MeterId = %i" % MeterId)
            for i in query.cursor.fetchall():
                LastMeteringId = LastMeteringId + [[i[0], i[1]]]
            return LastMeteringId
        finally:
            conn.close()

## API entrance to automatic turn off meter in standby mode
class LastestMetering(Resource):
    def get(self, MeterId, Iminimum):
        DataId = []
        relayIntermedValue = []
        relayValue = [0] *10
        iCount = 0
        try:
            conn = db_connect.connect()
            query = conn.execute("""SELECT Id from Metering WHERE MeterId = "%i" ORDER BY TimeStamp DESC Limit 5;""" % (MeterId))
            for i in query.cursor.fetchall():
                DataId = DataId + [i]
            for finddata in DataId:
                query = conn.execute("""SELECT Value from MeteringData WHERE MeteringId = "%i" AND Data = 2;""" %(finddata))
                for j in query.cursor.fetchall():
                    relayIntermedValue = relayIntermedValue + [j]
            relayValue[MeterId] = relayIntermedValue

            if(len(relayValue[MeterId]) == 5):
                for iValue in relayValue[MeterId]:
                    if ( iValue[0] < Iminimum):
                        iCount = iCount + 1
                    if iCount == 5:
                        return 'D'
                return "CORRENTE > QUE VALOR MINIMO"
            return "NAO HÁ AMOSTRAS SUFICIENTES"

        finally:
            conn.close()

## API entrance to return LastMeteringId from Metering table
class LastMeteringIdRasp(Resource):
    def get(self):
        LastMeteringId = []
        try:
            conn = db_connect.connect()
            query = conn.execute("Select Max(Id) from Metering")
            for i in query.cursor.fetchall():
                LastMeteringId = i[0]
            return LastMeteringId
        finally:
            conn.close()

## API entrance to insert data on database
class InserirDados(Resource):
    def get(self, Id, MeteringMeterId, Voltage, VoltageValue, Current, CurrentValue, PowerFactor, PowerFactorValue, Energy, EnergyValue, Reactive, ReactiveValue):
        sql =''
        try:
            conn = db_connect.connect()
            sql = "INSERT INTO MeteringData (MeteringId, Data, Value, MeteringMeterId) VALUES "
            sql = sql + "('%i','%i','%f','%i'), " % (Id, Voltage, VoltageValue, MeteringMeterId)
            sql = sql + "('%i','%i','%f','%i'), " % (Id, Current, CurrentValue, MeteringMeterId)
            sql = sql + "('%i','%i','%f','%i'), " % (Id, PowerFactor, PowerFactorValue, MeteringMeterId)
            sql = sql + "('%i','%i','%f','%i'), " % (Id, Energy, EnergyValue, MeteringMeterId)
            sql = sql + "('%i','%i','%f','%i');" % (Id, Reactive, ReactiveValue, MeteringMeterId)
            conn.execute(sql)
            #conn.commit()
            return 'Inserido Novos Dados'
        finally:
            conn.close()

## API entrance to insert new metering id
class InserirNovaMedicao(Resource):
    def get(self):
        try:
            conn = db_connect.connect()
            query = conn.execute("INSERT INTO Metering (MeterId, MeterFlag) VALUES (2,1)")
            return 'Inserido Nova Medição'
        finally:
            conn.close()

## API entrance to get meter flag (on or off)
class SelecionarFlags(Resource):
    def get(self, MeterId):
        Flags = []
        try:
            conn = db_connect.connect()
            #query = conn.execute("SELECT MeterFlag, UserFlag, Mode FROM Meter WHERE Id = (%i)" % MeterId)
            query = conn.execute("SELECT MeterFlag FROM Meter WHERE Id = (%i)" % MeterId)
            for i in query.cursor.fetchall():
                Flags = i[0]
                #Flags = Flags + [[i[0], i[1], i[2]]]
            return Flags
        finally:
            conn.close()

## API entrance to insert a metering flag
class InsereMeterFlagNovaMedicao(Resource):
    def get(self, MeterId, Flag):
        try:
            conn = db_connect.connect()
            query = conn.execute("INSERT INTO Metering (MeterId, MeterFlag) VALUES (%i, %i)" % (MeterId, Flag))
            return "Atualizado Flag no medidor %i para nova medicão" %(MeterId)
        finally:
            conn.close()

## API entrance to update a metering flag
class AtualizaMeterFlag(Resource):
    def get(self, Flag, MeterId):
        _Flag = Flag
        try:
            conn = db_connect.connect()
            if _Flag == 0:
                query = conn.execute("UPDATE Meter SET MeterFlag = 0 WHERE Id = %i" % (MeterId))
                MeterFlag = 0;
            else:
                query = conn.execute("UPDATE Meter SET MeterFlag = 1 WHERE Id = %i" % (MeterId))
                MeterFlag = 1;

            return "Status do medidor (MeterFlag) = %i " % (MeterFlag)
        finally:
            conn.close()


## Routing
# api.add_resource(MeterName, '/Meter/Name') # Route_1 - Return Meter name and address
api.add_resource(Meter, '/Meter')
api.add_resource(Metering, '/Metering')
api.add_resource(MeteringData, '/MeteringData/<int:LastId>')
api.add_resource(ChangeUserFlag, '/UserFlag1/<string:Address>')
api.add_resource(ChangeUserFlagTeste, '/UserFlag0/<string:Address>')
api.add_resource(ChangeMeterFlag1, '/MeterFlag1/<string:Address>')
api.add_resource(ChangeMeterFlag0, '/MeterFlag0/<string:Address>')
api.add_resource(NormalMode, '/Normal/<int:MeterId>')
api.add_resource(StandByMode, '/Standby/<int:MeterId>')
api.add_resource(Mode, '/Mode/<int:MeterId>')
api.add_resource(LastMeteringId, '/LastId/<int:MeterId>')
api.add_resource(LastestMetering, '/LastestMetering/Medidor<int:MeterId>/<float:Iminimum>')
api.add_resource(LastMeteringIdRasp, '/LastMeteringIdRasp')
api.add_resource(InserirDados, '/InserirDados/ID<int:Id>_METERINGMETERID<int:MeteringMeterId>__DATA<int:Voltage>_VALUE<float:VoltageValue>__DATA<int:Current>_VALUE<float:CurrentValue>__DATA<int:PowerFactor>_VALUE<float:PowerFactorValue>__DATA<int:Energy>_VALUE<float:EnergyValue>__DATA<int:Reactive>_VALUE<float:ReactiveValue>')  # Route_1 ##Resposta do site OK
api.add_resource(InserirNovaMedicao, '/InserirNovaMedicao')
api.add_resource(SelecionarFlags, '/SelecionarFlags<int:MeterId>')
api.add_resource(InsereMeterFlagNovaMedicao, '/InsereMeterFlagNovaMedicao<int:MeterId>Flag<int:Flag>')
api.add_resource(AtualizaMeterFlag, '/AtualizaMeterFlag=<int:Flag>_Meter=<int:MeterId>')

                    ## End Rest API ##


if __name__ == '__main__':
    app.run()