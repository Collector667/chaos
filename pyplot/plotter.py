from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.pyplot import title
name1 = 'Euler'
name2 = 'EulerCramer'
name3 = 'rk4'
name4 = 'midPoint'
name5 = 'CD'

# name1 = name5
BASE_DIR = Path(__file__).resolve().parent.parent

path1 = BASE_DIR / "csv" / name1
# path2 = BASE_DIR / "csv" / name2
# path3 = BASE_DIR / "csv" / name3
# path4 = BASE_DIR / "csv" / name4
# path5 = BASE_DIR / "csv" / name5

name = name1
path = BASE_DIR / "csv" / name

save_dir = BASE_DIR / "data" / "lab1"
save_dir.mkdir(parents=True, exist_ok=True)




# df1 = pd.read_csv(path1)
# df2 = pd.read_csv(path2)
# df3 = pd.read_csv(path3)
# df4 = pd.read_csv(path4)
# df5 = pd.read_csv(path5)

df = pd.read_csv(path)


# Вывод первых строк
print(df.head())
print(f"Всего строк: {len(df)}")

x = df["x"]
y = df["y"]
z = df["z"]
t = df["time"]

# print(df2.head())
# print(f"Всего строк: {len(df2)}")
#
# x2 = df2["x"]
# y2 = df2["y"]
# z2 = df2["z"]
# t2 = df2["time"]
#
# print(df3.head())
# print(f"Всего строк: {len(df3)}")
#
# x3 = df3["x"]
# y3 = df3["y"]
# z3 = df3["z"]
# t3 = df3["time"]
#
# print(df4.head())
# print(f"Всего строк: {len(df4)}")
#
# x4 = df4["x"]
# y4 = df4["y"]
# z4 = df4["z"]
# t4 = df4["time"]
#
# print(df5.head())
# print(f"Всего строк: {len(df5)}")
#
# x5 = df5["x"]
# y5 = df5["y"]
# z5 = df5["z"]
# t5 = df5["time"]



fig = plt.figure(figsize=(6, 6))

ax = fig.add_subplot(111, projection='3d')


ax.plot(x, z, y, c='green', label=name)

ax.set_xlabel("X")
ax.set_ylabel("Z")
ax.set_zlabel("Y")



ax.view_init(elev=60, azim=45)
plt.show()

S = name+'\nГрафик переменной X по времени'

plt.plot(t, x, c='blue', label=S)
plt.xlabel("Ось T")
plt.ylabel("Ось X")
S1 = save_dir / (name + "XT")
plt.savefig(S1)

plt.close()


S = name+'\nГрафик переменной Y по времени'

plt.plot(t, y, c='blue', label=S)
plt.xlabel("Ось T")
plt.ylabel("Ось Y")
S1 = save_dir / (name + "YT")
plt.savefig(S1)
plt.close()

S = name+'\nГрафик переменной Z по времени'

plt.plot(t, x, c='blue', label=S)
plt.xlabel("Ось T")
plt.ylabel("Ось Z")
S1 = save_dir / (name + "ZT")
plt.savefig(S1)

plt.close()



# dx = x-x3
# dy = y-y3
# dz = z-z3
#
# e = (dx**2+dy**2+dz**2)**0.5
# S = "Ошибка " + name
# plt.plot(t, e, c='red', label=S)
# plt.xlabel("Ось T")
# plt.ylabel("Глобальная ошибка")
# S1 = save_dir / (name + "ET")
# plt.savefig(S1)
# plt.close()
#
# print(e.values[-1])