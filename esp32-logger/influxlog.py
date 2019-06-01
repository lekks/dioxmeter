import operator
from functools import reduce
import requests


def parse_nmea(sentence):
    if sentence[0] != '$':
        return None
    sentence = sentence.strip('$ \r\n')
    nmeadata_cksum = sentence.split('*', 1)
    if len(nmeadata_cksum) != 2:
        return None
    calc_cksum = reduce(operator.xor, (ord(s) for s in nmeadata_cksum[0]), 0)
    if int(nmeadata_cksum[1], 16) != calc_cksum:
        return None
    return tuple(nmeadata_cksum[0].split(','))


class InfluxWriter:
    def __init__(self, url, db,  measurement, user, pswd, tags=None):
        self.db = db
        self.measurement = measurement
        self.url = url
        self.user = user
        self.pswd = pswd
        self.tags = tags

    @staticmethod
    def _influx_notime(measurement, values, tags=None):
        values_str = ','.join(["{}={}".format(key, str(val)) for key, val in values.items()])
        if tags:
            tags_str = ','.join(["{}={}".format(key, str(val)) if val else key for key, val in tags.items()])
        else:
            tags_str = None
        if tags_str:
            result = "{},{} {}".format(measurement, tags_str, values_str)
        else:
            result = "{} {}".format(measurement, values_str)
        return result

    def post(self, values):
        post_data = self._influx_notime(self.measurement, values, self.tags)
        params = {'db': self.db, 'u': self.user, 'p': self.pswd}
        return requests.post(self.url, params=params, data=post_data)

