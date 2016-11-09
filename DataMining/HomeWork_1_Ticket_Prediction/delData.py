import pickle


class Data:

    def __init__(self):
        return

with open('./data/TicketData', 'rb') as f:
    ticket = pickle.load(f)

r1 = ticket.red_1
r2 = ticket.red_2
r3 = ticket.red_3
r4 = ticket.red_4
r5 = ticket.red_5
r6 = ticket.red_6

count = len(r1)

data_set_r1 = [[] for i in range(count - 1)]
data_set_r2 = [[] for i in range(count - 1)]
data_set_r3 = [[] for i in range(count - 1)]
data_set_r4 = [[] for i in range(count - 1)]
data_set_r5 = [[] for i in range(count - 1)]
data_set_r6 = [[] for i in range(count - 1)]

for i in range(0, count - 1):
    data_set_r1[i] = [r2[i], r3[i], r4[i], r5[i], r6[i], r1[i + 1]]
    data_set_r2[i] = [r1[i], r3[i], r4[i], r5[i], r6[i], r2[i + 1]]
    data_set_r3[i] = [r1[i], r2[i], r4[i], r5[i], r6[i], r3[i + 1]]
    data_set_r4[i] = [r1[i], r2[i], r3[i], r5[i], r6[i], r4[i + 1]]
    data_set_r5[i] = [r1[i], r2[i], r3[i], r4[i], r6[i], r5[i + 1]]
    data_set_r6[i] = [r1[i], r2[i], r3[i], r4[i], r5[i], r6[i + 1]]

data_set = {'red_1': data_set_r1,
            'red_2': data_set_r2,
            'red_3': data_set_r3,
            'red_4': data_set_r4,
            'red_5': data_set_r5,
            'red_6': data_set_r6,
            }

with open('./data/DecisionTreeDataSet', 'wb') as f:
    pickle.dump(data_set, f)
