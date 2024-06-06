#include "fimd.h"
#include <NTL/ZZX.h>
#include "../utils/common.h"
#include "../utils/timeUtil.h"

using namespace std;

void FIMD::set_evaluation_points(vector<long> new_evaluation_points)
{
    evaluation_points = new_evaluation_points;
    generate_lagrange_basis();
}

void FIMD::generate_lagrange_basis()
{
    lagrange_basis.clear();
    for (int i = 0; i < evaluation_points.size(); i++) {
        vector<long> component;
        component.push_back(1);
        for (int j = 0; j < evaluation_points.size(); j++) {
            if (j == i)
                continue;
            component.push_back(component.back());
            for (int k = component.size() - 2; k > 0; k--)
                component[k] = (component[k - 1] + (p - component[k]) * evaluation_points[j]) % p;
            component[0] = ((p - component[0]) * evaluation_points[j]) % p;
        }

        long inv = 1;
        for (int j = 0; j < evaluation_points.size(); j++) {
            if (i == j)
                continue;
            (inv *= modInverse((p + evaluation_points[i] - evaluation_points[j]) % p, p)) %= p;
        }

        for (int j = 0; j < component.size(); j++)
            (component[j] *= inv) %= p;
        lagrange_basis.push_back(component);
    }
}

// Lagrange Interpolation
vector<long> FIMD::lagrange_interpolation(vector<long> f)
{
    vector<long> coeff(f.size(), 0);
    for (int i = 0; i < f.size(); i++) {
        for (int j = 0; j < lagrange_basis[i].size(); j++)
            (coeff[j] += lagrange_basis[i][j] * f[i]) %= p;
    }
    return coeff;
}

void FIMD::encode(helib::Ptxt<helib::BGV> &ptxt, vector<long> values, long index)
{
    NTL::ZZX poly = ptxt[index].getData();
    NTL::clear(poly);
    vector<long> corresponding_field_element = lagrange_interpolation(values);
    cout << "The polynomial after Lagrange interpolation is: ";
    for (long xx : corresponding_field_element)
        cout << xx << " ";
    cout << "\n";

    for (long j = 0; j < corresponding_field_element.size(); j++) {
        NTL::SetCoeff(poly, j, corresponding_field_element[j]);
    }

    ptxt[index] = poly;
}

void FIMD::decode(vector<long> &values, helib::Ptxt<helib::BGV> ptxt, long index)
{
    NTL::ZZX poly = ptxt[index].getData();
    long poly_deg = NTL::deg(poly);

    if (poly_deg == -1) // Polynomial equal to 0
    {
        for (long j = 0; j < evaluation_points.size(); j++) {
            values[j] = 0;
        }
        return;
    }

    vector<long> poly_long(poly_deg + 1, 0);
    for (long i = 0; i <= poly_deg; i++) {
        NTL::conv(poly_long[i], NTL::coeff(poly, i)); // convert ZZ to long
    }

    for (long j = 0; j < evaluation_points.size(); j++) {
        values[j] = 0;
        long curr = 1;
        for (long i = 0; i <= poly_deg; i++) {
            (values[j] += curr * poly_long[i]) %= p;
            (curr *= evaluation_points[j]) %= p;
        }
    }
}

void FIMD::q_linearize(helib::Ctxt &ctxt)
{
    long deg = q_linearized_coeff.size();
    long sqrt_deg = ceil(sqrt(deg));

    vector<helib::Ctxt> small_frob;
    small_frob.push_back(ctxt);
    for (int i = 1; i < sqrt_deg; i++) {
        small_frob.push_back(small_frob[i - 1]);
        small_frob[i].frobeniusAutomorph((long)1);
    }

    for (long j = 0; j <= sqrt_deg; j++) {
        if (j * sqrt_deg >= out_degree)
            continue;
        helib::Ctxt small_tot = small_frob[0];
        for (int i = 0; i < sqrt_deg; i++) {
            if (j * sqrt_deg + i >= out_degree)
                continue;
            if (i == 0) {
                small_tot.multByConstant(q_linearized_coeff[j * sqrt_deg + i]);
            } else {
                helib::Ctxt dummy_ctxt = small_frob[i];
                dummy_ctxt.multByConstant(q_linearized_coeff[j * sqrt_deg + i]);
                small_tot.addCtxt(dummy_ctxt);
            }
        }

        small_tot.frobeniusAutomorph(j * sqrt_deg);
        if (j == 0)
            ctxt = small_tot;
        else
            ctxt.addCtxt(small_tot);
    }
}

void FIMD::multByConstant(helib::Ctxt &ctxt, helib::Ptxt<helib::BGV> ptxt)
{
    ctxt.multByConstant(ptxt);
    cout << "Performing q-linearization:\n";
    TIME(q_linearize(ctxt));
}

void FIMD::multByCtxt(helib::Ctxt &ctxt, helib::Ctxt other_ctxt)
{
    ctxt.multiplyBy(other_ctxt);
    cout << "Performing q-linearization:\n";
    TIME(q_linearize(ctxt));
}
