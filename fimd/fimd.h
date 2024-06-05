#include <iostream>
#include <vector>

#include <helib/helib.h>

class FIMD
{
public:
    FIMD(long new_p, long new_in_degree, long new_out_degree): p(new_p), in_degree(new_in_degree), out_degree(new_out_degree)
    {}

    void set_evaluation_points(std::vector<long> x);
    void generate_lagrange_basis();
    std::vector<long> lagrange_interpolation(std::vector<long> f);

    void encode(helib::Ptxt<helib::BGV> &ptxt, std::vector<long> values, long index);
    void decode(std::vector<long> &values, helib::Ptxt<helib::BGV> ptxt, long index);

    void q_linearize(helib::Ctxt &ctxt);
    void multByConstant(helib::Ctxt &ctxt, helib::Ptxt<helib::BGV> ptxt);
    void multByCtxt(helib::Ctxt &ctxt, helib::Ctxt other_ctxt);


    std::vector<helib::Ptxt<helib::BGV>> q_linearized_coeff;

private:
    long p;
    long in_degree;
    long out_degree;
    std::vector<std::vector<long>> lagrange_basis;
    std::vector<long> evaluation_points;
};