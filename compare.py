#!/usr/bin/env python3

import os
import sys
import subprocess
import re
import fileinput
import operator

def check_results():
    p1 = open("Producer_RED.txt", "r")
    p2 = open("Producer_BLUE.txt", "r")
    c = open("Consumer.txt", "r")

    p1data = p1.read().rstrip().split('\n')
    filter(None, p1data)
    p2data = p2.read().rstrip().split('\n')
    filter(None, p2data)

    p1items=[]
    for v in p1data: # v is a single entry
        a, b = v.split()
        p1items.append((a, int(b)))
    for v in p2data:
        a, b = v.split()
        p1items.append((a, int(b)))

    producerList= sorted(p1items, key=lambda x: (x[1],x[0]))

    cdata = c.read().rstrip().split('\n')
    filter(None, cdata)
    citems=[]
    for v in cdata:
        a, b = v.split()
        citems.append((a, int(b)))

    consumerList = sorted(citems, key=lambda x: (x[1],x[0]))

    if len(consumerList) != len(producerList):
        print("Error The data size is not equal")
        sys.exit(-1)

    # Now match if both data are equal:
    for i in range(len(consumerList)):
        if consumerList[i][0] != producerList[i][0] or consumerList[i][1] != producerList[i][1]:
            print("Data Mismatch.")
            print("Producer ", producerList[i], " Consumer ", consumerList[i])
            sys.exit(-1)

    print("Results Match")
    p1.close()
    p2.close()
    c.close()
    os.remove("Producer_RED.txt")
    os.remove("Producer_BLUE.txt")
    os.remove("Consumer.txt")


if __name__ == '__main__':

        check_results()


