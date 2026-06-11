import matplotlib.pyplot as plt #used to give a graphical repreentation of the coparison in timings between malloc and malloc that
#ive built 
import csv
labels = []
times = []
with open("results.csv") as file:
    reader = csv.DictReader(file)
    for row in reader:
        labels.append(row["allocator"])
        times.append(float(row["time"]))
plt.bar(labels, times, color=["blue", "orange"])
plt.title("my_malloc vs standard malloc — 10,000 allocations")
plt.ylabel("Time (seconds)")
plt.show()