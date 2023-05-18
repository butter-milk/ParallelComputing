import matplotlib.pyplot as plt
from tabulate import tabulate
import os

def createTable(file, filename):
    filenamedata= filename.split('_')
    print("n = " [3] )
    print("\\begin\{tabular\}")
    colNames = ["Time (s)", "GFlops/s"]
    with open(file, "r") as f:
        for line in f:
            data = line.split(' ')









for filename in os.listdir("../results"):
    f = os.path.join("../results", filename)
    createTable(f, filename)