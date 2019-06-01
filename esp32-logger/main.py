import machine

from influxlog import InfluxWriter, parse_nmea

URL = ""
DB = ""
USER = ""
PASS = ""
MEASUREMENT = "hut_env"
PLACE = "home"


def post_value(logger, param, value):
    answ = logger.post({})
    if 200 <= answ.status_code < 300:
        print("Sent {}={}".format(param, value))
        return True
    else:
        print(answ)
        return False


if __name__ == '__main__':
    logger = InfluxWriter(URL, DB, MEASUREMENT, USER, PASS)
    in_port = machine.UART(2, baudrate=9600, rx=16, tx=17, timeout=10)
    hum_sent = None
    co2_sent = None
    while True:
        nmea_line = in_port.readline()
        in_data = parse_nmea(nmea_line)
        print(nmea_line, in_data)
        if in_data:
            param, value = in_data
            if param == "CO2" and value and value != co2_sent:
                if post_value(logger, "CO2", value):
                    co2_sent = value
            elif param == "HUM" and value and value != hum_sent:
                if post_value(logger, "Humidity", value):
                    hum_sent = value
            elif param == "VER":
                print("Dioxmeter version {}".format(value))
