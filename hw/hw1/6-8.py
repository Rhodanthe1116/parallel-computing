# t = 100000
# n-byte, n = 8
# Execution time 0.142340
# Avg latency + (n / bandwith) = 0.0000007117

# t = 100000
# n-byte, n = 4
# Execution time 0.135645
# Avg latency + (n / bandwith) = 0.0000006782

# latency + (8 / bandwith) = 0.0000007117
# latency + (4 / bandwith) = 0.0000006782

time1 = 0.0000007117
n1 = 8
time2 = 0.0000006782
n2 = 4
time3 = 0.0000006040
n3 = 4

# time1 - time2 = (n1+n2)/b

def calLB(time1, n1, time2, n2):
    b = (n1+n2) / (time1-time2)
    l = time1 - (n1/b)

    return l, b
    
l, b = calLB(time1, n1, time2, n2)
print(l, b)

b = (n2+n3) / (time2 - time3)
l = time2 - (n2 / b)
print(l, b)

# cpu1
l, b = calLB(0.0000007299, 8, 0.0000007752, 4)
print(l, b)
# b = (n1+n2) / (time1 - time2)
