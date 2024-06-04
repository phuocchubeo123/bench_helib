## In this Sage code, I will try to create RMFE with the following parameters
## Prime plaintext p
## Polynomial degree m
## One slot has d
## Degree in
## Degree out

p = 257
d = 16
degree_in = 8
degree_out = 2*degree_in - 1

F.<a> = FiniteField(p^d)
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

def q_linearized(u):
    ## To implement this in FIMD, we will try to use Patterson-Stockmeyer trick but for q-linearized matrix
    res = 0
    for i in range(degree_out):
        res += final_transformation[0][i] * (u^(p^i))
    return res


# Testing if the q-linearized transformation is correct
u = a^9
print(q_linearized(u))


print()
print()
print('TO COPY PASTE INTO PARAMS FILE')
for i in final_transformation[0]:
    print(' '.join(list(map(str, i.list()))))
