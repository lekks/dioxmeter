import unittest
from influxlog import parce_line, InfluxWriter


class TestParser(unittest.TestCase):

    def test_parse(self):
        lines = ["$VER,1.3*41", "$CO2,410*27", "$CO2,409*2F", "$HUM,33*7C", "$CO2,395*2D", "$CO2,410*27", "$HUM,32*7D"]
        expect = [("VER", "1.3"), ("CO2", "410"), ("CO2", "409"), ("HUM", "33"), ("CO2", "395"), ("CO2", "410"),
                  ("HUM", "32")]
        for line, exp in zip(lines, expect):
            res = tuple(parce_line(line))
            self.assertEqual(res, exp)

    def test_parse_neg(self):
        lines = ["VER,1.3*41", "$CO2,410*28", "$CO2,4092F", "$HUM,43*7C"]
        for line in lines:
            res = parce_line(line)
            self.assertIsNone(res)


class TestInflux(unittest.TestCase):

    def test_tags(self):
        res = InfluxWriter._influx_notime("hut_env", {"tempr": 12, "hum": 24}, {"hut": "home", "test": None})
        self.assertEqual(res, "hut_env,hut=home,test tempr=12,hum=24")

    def test_nnoags(self):
        res = InfluxWriter._influx_notime("hut_env", {"tempr": 12, "hum": 24})
        self.assertEqual(res, "hut_env tempr=12,hum=24")

if __name__ == '__main__':
    unittest.main()