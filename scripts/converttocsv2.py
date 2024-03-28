import json, sys

data_head = [
    "adc_mq135", 
    "adc_mq136", 
    "adc_mq137", 
    "adc_mq138",
    "adc_mq2", 
    "adc_mq3",
    "adc_tgs822",  
    "adc_tgs2620", 
    "status"
]

def convertFile(filename):
    file = open(filename)
    buffer_out = ""

    header = ",".join(data_head)

    buffer_out += header + "\n"

    for f in file.readlines():
        jdata = json.loads(f)
        out = []

        for name in data_head:
            try:
                out.append(str(jdata[name]))
            except:
                continue

        out_s = ",".join(out)
        buffer_out += out_s + "\n"

    file.close()

    fileo = open(filename + ".csv", "+w")
    fileo.write(buffer_out)
    fileo.close()

if len(sys.argv) < 2:
    print("need filename")
    exit(1)

convertFile(sys.argv[1])