## In this Sage code, I will try to create RMFE with the following parameters
## Prime plaintext p
## Polynomial degree m
## One slot has d
## Degree in
## Degree out

import math

p = 257
d = 16
degree_in = 8
degree_out = 2*degree_in - 1

K.<y> = GF(p)[]
F.<a> = FiniteField(p^d, modulus = y^16+3)
Zp = Integers(p)
R.<x> = PolynomialRing(Zp)


## List out all evaluation points:
evaluation_points = [Zp(i) for i in range(degree_in)]
## Compute the polynomial that takes all evaluation points as root
poly = Zp(1)
print(poly)
for i in range(degree_in):
    poly = poly * (x - evaluation_points[i])
    

## Create basis of the out field. For now keep it simple 1, a, a^2, ...
basis_list = []
for i in range(degree_out):
    basis_list.append(a^i)
basis = matrix(basis_list).transpose()

## Create Moore matrix. 
moore_list = []
for i in range(degree_out):
    moore_list.append([])
    for j in range(degree_out):
        moore_list[-1].append((a^j)^(p^i))
                              
moore = matrix(moore_list)
moore_inverse = moore.inverse_of_unit()


## Create the recode linear transformation, which is basically reducing mod poly

powers_reduced = [x^i % poly for i in range(degree_out)]
print('powers_reduced:')
for uu in powers_reduced:
    print(uu)

linear_trans_list = []

for i in range(degree_out):
    coefficients = powers_reduced[i].list()
    linear_trans_list.append(coefficients + [0 for j in range(degree_out - len(coefficients))])


linear_trans = matrix(linear_trans_list)

final_transformation =  (linear_trans * basis).transpose() * moore_inverse

print('Final q-linearized transformation coefficients')
for i in final_transformation[0]:
    print(i)

def find_discrete_log_frob(u, exponent_of_p):
    exponent = p^exponent_of_p
    ref_element = 0
    for i in range(d):
        ref_element += a^i
    ref_element = ref_element ^ exponent
    
    l1 = u.list()
    l1 = l1 + [0 for i in range(d-len(l1))]
    l2 = ref_element.list()
    l3 = [l1[i] / l2[i] for i in range(d)]
    
    res = 0
    for i in range(d):
        res += a^i * l3[i]
    
    return res

def q_linearized(u):
    ## To implement this in FIMD, we will try to use Patterson-Stockmeyer trick but for q-linearized matrix
    res = 0
    for i in range(degree_out):
        res += final_transformation[0][i] * (u^(p^i))
    return res

def q_linearized2(u):
    sqrt_deg = math.ceil(math.sqrt(degree_out))
    res = 0
    for j in range(sqrt_deg + 1):
        if j * sqrt_deg >= degree_out:
            break
        small_res = 0
        for i in range(sqrt_deg):
            if j * sqrt_deg + i >= degree_out:
                break
            small_res += (u^(p^i)) * find_discrete_log_frob(final_transformation[0][j*sqrt_deg + i], j*sqrt_deg)
        res += small_res^(p^(j*sqrt_deg))
    return res
            


# Testing if the q-linearized transformation is correct
for i in range(degree_out):
    print(x^i % poly)
    print(q_linearized(a^i))
    print(q_linearized2(a^i))


print()
print()
print('TO COPY PASTE INTO PARAMS FILE')
sqrt_deg = math.ceil(math.sqrt(degree_out))
for j in range(sqrt_deg + 1):
    if j * sqrt_deg >= degree_out:
        break
    for i in range(sqrt_deg):
        if j * sqrt_deg + i >= degree_out:
            break
        u = find_discrete_log_frob(final_transformation[0][j*sqrt_deg + i], j * sqrt_deg)
        print(' '.join(list(map(str, u.list()))))
