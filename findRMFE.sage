## In this Sage code, I will try to create RMFE with the following parameters
## Prime plaintext p = 31
## Polynomial degree m = 1261
## One slot has d = 48
## Degree in = 15
## Degree out = 29

p = 31
d = 48
degree_in = 15
degree_out = 2*degree_in - 1

F.<a> = FiniteField(p^d)
R = PolynomialRing(F, 'x')

## A little note on RMFE. We will tend to choose the evaluation points to be roots of simple equations.
## For example if q = 5, choosing \alpha = [1, 4], they will be roots of x^2-1.
## That means the trasformation will be: 1 -> 1, a -> a, a^2 -> 1.
## This will make life much easier, but there maybe more efficient transformations, which yield sparse q-linearized matrices.

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

linear_trans_list = []
for i in range(degree_in):
    linear_trans_list.append([0 for i in range(degree_out)])
    linear_trans_list[-1][i] = 1

for i in range(degree_in, degree_out):
    linear_trans_list.append([0 for i in range(degree_out)])
    linear_trans_list[-1][i-degree_in] = 1
    
linear_trans = matrix(linear_trans_list)


final_transformation = (linear_trans * basis).transpose() * moore_inverse
#print(final_transformation.transpose())

for i in final_transformation[0]:
    #print(i)
    i_list = i.list()
    print(' '.join(list(map(str, i_list))))

def q_linearized(u):
    ## To implement this in FIMD, we will try to use Patterson-Stockmeyer trick but for q-linearized matrix
    res = 0
    for i in range(degree_out):
        res += final_transformation[0][i] * (u^(p^i))
    return res


# Testing if the q-linearized transformation is correct
u = a^18
print(q_linearized(u))
