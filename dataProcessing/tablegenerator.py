import matplotlib.pyplot as plt
from tabulate import tabulate
import os
import itertools
import numpy as np
import pandas as pd

# dataSEC = list([threads: int, n: int, iterations: int, seconds: float])
dataSEC = []
# dataGFLOPS = list([threads: int, n: int, iterations: int, Gflops/s: float])
dataGFLOPS = []

# dataGFLOPS = list([threads: int, n: int, iterations: int, seconds: float, Gflops/s: float])
dataCSV = []


def createTable(file, filename):
    filenamedata = filename.split("_")
    with open("latexTables.txt", "a+") as g:
        g.write(
            "threads = "
            + filenamedata[4][:-4]
            + "n = "
            + filenamedata[2]
            + " iterations = "
            + filenamedata[3]
            + "\\\\\n"
        )
        g.write("\\begin{tabular}{c|c}" + "\n")
        g.write("Time (s) & GFlops/s\\\\" + "\n")
        with open(file, "r") as f:
            for line in [l for l in f]:
                data = line.split(" ")
                g.write(
                    data[0][: min(5, len(data[0]))]
                    + " & "
                    + data[1][: min(5, len(data[1]))]
                    + "\\\\"
                )
        g.write("\\end{tabular}\\\\" + "\n")


def dataToLists(file, filename):
    with open(file, "r") as f:
        for line in f:
            data = line.split(" ")
            dataSEC.append(
                [
                    int(filename.split("_")[4][:-4]),
                    int(filename.split("_")[2]),
                    int(filename.split("_")[3]),
                    float(data[0]),
                ]
            )
            dataGFLOPS.append(
                [
                    int(filename.split("_")[4][:-4]),
                    int(filename.split("_")[2]),
                    int(filename.split("_")[3]),
                    float(data[1]),
                ]
            )
            dataCSV.append([
                int(filename.split("_")[4][:-4]),
                    int(filename.split("_")[2]),
                    int(filename.split("_")[3]),
                    float(data[0]),
                    float(data[1]),
            ])


def createPlots():
    # CreateLinePlots SECONDS
    for threadno in [x[0] for x in dataSEC]:
        dataSec = [x[1:] for x in dataSEC]
        dataGFlops = [x[1:] for x in dataGFLOPS]
        for iterations, group in itertools.groupby(dataSec, lambda x: x[1]):
            datapoints = [[int(x[0]), float(x[2])] for x in list(group)]
            datapointsMean = [
                (x[0], np.mean([y[1] for y in datapoints if y[0] == x[0]]))
                for x in datapoints
            ]
            datapointsMin = [
                (x[0], min([y[1] for y in datapoints if y[0] == x[0]]))
                for x in datapoints
            ]
            datapointsMax = [
                (x[0], max([y[1] for y in datapoints if y[0] == x[0]]))
                for x in datapoints
            ]
            plt.plot(
                [x[0] for x in datapointsMean],
                [x[1] for x in datapointsMean],
                label="iterations= " + str(iterations) + " mean",
                marker="o",
            )
            plt.plot(
                [x[0] for x in datapointsMin],
                [x[1] for x in datapointsMin],
                label="iterations= " + str(iterations) + " minimum",
                marker="o",
            )
            plt.plot(
                [x[0] for x in datapointsMax],
                [x[1] for x in datapointsMax],
                label="iterations= " + str(iterations) + " maximum",
                marker="o",
            )
        plt.legend()
        plt.title("Runtime of mpi_stencil_opt with " + str(threadno) + " threads")
        plt.xlabel("Data (n)")
        plt.ylabel("Time (s)")
        plt.show()

        for iterations, group in itertools.groupby(dataGFlops, lambda x: x[1]):
            datapoints = [[int(x[0]), float(x[2])] for x in list(group)]
            datapointsMean = [
                (x[0], np.mean([y[1] for y in datapoints if y[0] == x[0]]))
                for x in datapoints
            ]
            datapointsMin = [
                (x[0], min([y[1] for y in datapoints if y[0] == x[0]]))
                for x in datapoints
            ]
            datapointsMax = [
                (x[0], max([y[1] for y in datapoints if y[0] == x[0]]))
                for x in datapoints
            ]
            plt.plot(
                [x[0] for x in datapointsMean],
                [x[1] for x in datapointsMean],
                label="iterations= " + str(iterations) + " mean",
                marker="o",
            )
            plt.plot(
                [x[0] for x in datapointsMin],
                [x[1] for x in datapointsMin],
                label="iterations= " + str(iterations) + " minimum",
                marker="o",
            )
            plt.plot(
                [x[0] for x in datapointsMax],
                [x[1] for x in datapointsMax],
                label="iterations= " + str(iterations) + " maximum",
                marker="o",
            )
        plt.legend()
        plt.title("Operations of mpi_stencil_opt with " + str(threadno) + " threads")
        plt.xlabel("Data (n)")
        plt.ylabel("Operations (Gflops/s)")
        plt.show()

    return

def createCSV():
    pd.DataFrame(dataCSV).to_csv('data.csv')    


# DO STUFF WITH DATA
for filename in os.listdir("../parallel_programs-master/results"):
    f = os.path.join("../parallel_programs-master/results", filename)
    createTable(f, filename)
    dataToLists(f, filename)
createCSV()
#createPlots()
