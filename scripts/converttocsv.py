import json

def convertFile(filename):
    file = open("./data/" + filename + ".txt")
    buffer_out = ""

    header = "adc_mq135, adc_mq137, adc_mq138, adc_mq3, status"

    buffer_out += header + "\n"

    for f in file.readlines():
        jdata = json.loads(f)
        out = []
        
        out.append(str(jdata["adc_mq135"]))
        out.append(str(jdata["adc_mq137"]))
        out.append(str(jdata["adc_mq138"]))
        out.append(str(jdata["adc_mq3"]))
        out.append(str(jdata["status"]))

        out_s = ",".join(out)
        buffer_out += out_s + "\n"

    file.close()

    fileo = open("./data/" + filename + ".csv", "+w")
    fileo.write(buffer_out)
    fileo.close()

filenames = [
    "arabica1",
    "arabica2",
    "arabica3",
    "arabica4",
    "arabica5",
    "arabica6",
    "arabica7",
    "arabica8",
    "arabica9"
    ]

for fn in filenames:
    convertFile(fn)