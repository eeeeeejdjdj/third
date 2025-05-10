def find_S(examples):
    h = ['$'] * len(attributes)  # 初始化最特殊假设
    for e in examples:
        if not e[1]:  # 跳过负例
            continue
        for i in range(len(h)):
            if h[i] == '$':
                h[i] = e[0][i]
            elif h[i] != e[0][i]:
                h[i] = '?'
    return h

def agree(h, e, elabel):
    tmp = True
    for i in range(len(h)):
        if h[i] == e[i] or h[i] == '?':
            continue
        else:
            tmp = False
    return tmp == elabel

def is_special_than(h1, h2, strict=False):
    for i in range(len(h1)):
        if h1[i] == h2[i] or h2[i] == '?':
            continue
        else:
            return False
    return True

def is_general_than(h1, h2, strict=False):
    return is_special_than(h2, h1, strict)

def min_generalize(h, e):
    new_h = h.copy()
    for i in range(len(new_h)):
        if new_h[i] == '$':
            new_h[i] = e[i]
        elif new_h[i] != e[i]:
            new_h[i] = '?'
    return new_h

def min_specialize(h):
    generate_h = []
    for i in range(len(h)):
        if h[i] == '?':
            for val in attributes[i]:
                new_h = h.copy()
                new_h[i] = val
                generate_h.append(new_h)
    return generate_h

def generate_SG(examples):
    empty_h = ['$'] * len(attributes)
    full_h = ['?'] * len(attributes)
    S = [empty_h.copy()]
    G = [full_h.copy()]
    cont=0
    for e in examples:
        x, label = e[0], e[1]
        if label: 
            G = [g for g in G if agree(g, x, True)]
            new_S = []
            for s in S:
                generalized = min_generalize(s, x)
                if generalized not in new_S:
                    new_S.append(generalized)
            S = [s for s in new_S if any(is_general_than(g, s) for g in G)]
        else:  # 负例
            S = [s for s in S if agree(s, x, False)]
            new_G = []
            for g in G:
                if agree(g, x, True): 
                    for h_spec in min_specialize(g):
                        if not agree(h_spec, x, True) and any(is_special_than(s, h_spec) for s in S):
                            new_G.append(h_spec)
            unique_G = []
            for g in new_G:
                if g not in unique_G:
                    unique_G.append(g)
            G = unique_G
        print(f"S{cont}: {S}")
        print(f"G{cont}: {G}")
        cont+=1
    return S, G

def generate_VS(S,G):
    VS=[]
    for g in G:
        ori_g=g.copy()
        for s in S:
            for i in range(len(s)):
                if g[i] == '?' and s[i] != '?':
                    g[i]=s[i]
                    if g not in VS:
                        VS.append(g)
                    g=ori_g.copy()
    return VS


if __name__=="__main__":
    examples = [
[['Sunny', 'Warm', 'Normal', 'Strong', 'Warm', 'Same'], True],
[['Sunny', 'Warm', 'High', 'Strong', 'Warm', 'Same'], True],
[['Rainy', 'Cold', 'High', 'Strong', 'Warm', 'Change'], False],
[['Sunny', 'Warm', 'High', 'Strong', 'Cool', 'Change'], True]
]


attributes = [
    ['Sunny', 'Rainy'],
    ['Warm', 'Cold'],
    ['Normal', 'High'],
    ['Strong', 'Light'],
    ['Warm', 'Cool'],
    ['Same', 'Change']
]

empty_h=['$']*len(attributes)
full_h=['?']*len(attributes)

print('''--Find_s--''')
most_special_h=find_S(examples)
print(f"most_special_h: {most_special_h}")
print('''--end of find_s--''')

print("---Candidate-Elimination and Version-space---")
S,G=generate_SG(examples)
print(f"S:{S}")
VS=generate_VS(S,G)
for h in VS:
    print(h)
print(f"G:{G}")
print('''---end of Candidate-Elimination and Version-space---''')
