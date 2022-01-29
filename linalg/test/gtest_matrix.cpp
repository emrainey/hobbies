
#include <gtest/gtest.h>
#include <linalg/linalg.hpp>
#include <basal/basal.hpp>
#include <vector>
#include "linalg/gtest_helper.hpp"

TEST(MatrixTest, NormalConstructions) {
    using namespace linalg;
    matrix m0(2,2);
    matrix m1(3,3);
    matrix m2(4,4);
    matrix m3(5,5);
}

TEST(MatrixTest, InvalidConstructors) {
    using namespace linalg;
    ASSERT_THROW(matrix m0(0, 1), basal::exception);
    ASSERT_THROW(matrix m0(1, 0), basal::exception);
}

TEST(MatrixTest, ListInitializer) {
    using namespace linalg;
    matrix m{{ {1,2}, {3,4} }};
    ASSERT_EQ(2, m.cols);
    ASSERT_EQ(2, m.rows);
    ASSERT_DOUBLE_EQ(1, m[0][0]);
    ASSERT_DOUBLE_EQ(2, m[0][1]);
    ASSERT_DOUBLE_EQ(3, m[1][0]);
    ASSERT_DOUBLE_EQ(4, m[1][1]);
}

TEST(MatrixTest, ArrayConstructor) {
    using namespace linalg;
    double a1[] = {1,2,3,4};
    matrix m2a(2, 2, a1);
    ASSERT_EQ(2, m2a.cols);
    ASSERT_EQ(2, m2a.rows);
    ASSERT_DOUBLE_EQ(1, m2a[0][0]);
    ASSERT_DOUBLE_EQ(2, m2a[0][1]);
    ASSERT_DOUBLE_EQ(3, m2a[1][0]);
    ASSERT_DOUBLE_EQ(4, m2a[1][1]);

    // only the row accessor has a throw, the other is just a double[]
    ASSERT_THROW(m2a[7][1] += 0.0, basal::exception);

    double a2[2][2] = {{5,6}, {7,8}};
    matrix m2(a2);
    ASSERT_EQ(2, m2.cols);
    ASSERT_EQ(2, m2.rows);
    ASSERT_DOUBLE_EQ(5, m2[0][0]);
    ASSERT_DOUBLE_EQ(6, m2[0][1]);
    ASSERT_DOUBLE_EQ(7, m2[1][0]);
    ASSERT_DOUBLE_EQ(8, m2[1][1]);

    double a3[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    matrix m3(a3);
    ASSERT_EQ(3, m3.cols);
    ASSERT_EQ(3, m3.rows);
    ASSERT_DOUBLE_EQ(1, m3[0][0]);
    ASSERT_DOUBLE_EQ(2, m3[0][1]);
    ASSERT_DOUBLE_EQ(3, m3[0][2]);
    ASSERT_DOUBLE_EQ(4, m3[1][0]);
    ASSERT_DOUBLE_EQ(5, m3[1][1]);
    ASSERT_DOUBLE_EQ(6, m3[1][2]);
    ASSERT_DOUBLE_EQ(7, m3[2][0]);
    ASSERT_DOUBLE_EQ(8, m3[2][1]);
    ASSERT_DOUBLE_EQ(9, m3[2][2]);

    double a4[4][4] = {{1,2,3,4},{5,6,7,8},{9,0,1,2},{3,4,5,6}};
    matrix m4(a4);
    ASSERT_DOUBLE_EQ(1, m4[0][0]);
    ASSERT_DOUBLE_EQ(2, m4[0][1]);
    ASSERT_DOUBLE_EQ(3, m4[0][2]);
    ASSERT_DOUBLE_EQ(4, m4[0][3]);

    ASSERT_DOUBLE_EQ(5, m4[1][0]);
    ASSERT_DOUBLE_EQ(6, m4[1][1]);
    ASSERT_DOUBLE_EQ(7, m4[1][2]);
    ASSERT_DOUBLE_EQ(8, m4[1][3]);

    ASSERT_DOUBLE_EQ(9, m4[2][0]);
    ASSERT_DOUBLE_EQ(0, m4[2][1]);
    ASSERT_DOUBLE_EQ(1, m4[2][2]);
    ASSERT_DOUBLE_EQ(2, m4[2][3]);

    ASSERT_DOUBLE_EQ(3, m4[3][0]);
    ASSERT_DOUBLE_EQ(4, m4[3][1]);
    ASSERT_DOUBLE_EQ(5, m4[3][2]);
    ASSERT_DOUBLE_EQ(6, m4[3][3]);
}

TEST(MatrixTest, CopyConstructor) {
    using namespace linalg;
    matrix m{{ {1,2}, {3,4} }};
    matrix n(m);
    ASSERT_EQ(2, n.cols);
    ASSERT_EQ(2, n.rows);
    ASSERT_DOUBLE_EQ(1, n[0][0]);
    ASSERT_DOUBLE_EQ(2, n[0][1]);
    ASSERT_DOUBLE_EQ(3, n[1][0]);
    ASSERT_DOUBLE_EQ(4, n[1][1]);
}

TEST(MatrixTest, MoveConstructor) {
    using namespace linalg;
    matrix m{{ {1,2}, {3,4} }};
    matrix n(std::move(m));
    ASSERT_EQ(2, n.cols);
    ASSERT_EQ(2, n.rows);
    ASSERT_DOUBLE_EQ(1, n[0][0]);
    ASSERT_DOUBLE_EQ(2, n[0][1]);
    ASSERT_DOUBLE_EQ(3, n[1][0]);
    ASSERT_DOUBLE_EQ(4, n[1][1]);
}

TEST(MatrixTest, CopyAssignment) {
    using namespace linalg;
    matrix m{{ {1,2}, {3,4} }};
    matrix n = m;
    ASSERT_EQ(2, n.cols);
    ASSERT_EQ(2, n.rows);
    ASSERT_DOUBLE_EQ(1, n[0][0]);
    ASSERT_DOUBLE_EQ(2, n[0][1]);
    ASSERT_DOUBLE_EQ(3, n[1][0]);
    ASSERT_DOUBLE_EQ(4, n[1][1]);
    matrix q(n.copy());
    ASSERT_DOUBLE_EQ(1, q[0][0]);
    ASSERT_DOUBLE_EQ(2, q[0][1]);
    ASSERT_DOUBLE_EQ(3, q[1][0]);
    ASSERT_DOUBLE_EQ(4, q[1][1]);
}

TEST(MatrixTest, MoveAssignment) {
    using namespace linalg;
    matrix m{{ {1,2}, {3,4} }};
    matrix n = std::move(m);
    ASSERT_EQ(2, n.cols);
    ASSERT_EQ(2, n.rows);
    ASSERT_DOUBLE_EQ(1, n[0][0]);
    ASSERT_DOUBLE_EQ(2, n[0][1]);
    ASSERT_DOUBLE_EQ(3, n[1][0]);
    ASSERT_DOUBLE_EQ(4, n[1][1]);
}

TEST(MatrixTest, EqualityAndInEquality) {
    using namespace linalg;
    matrix m{{{ 4,  6, 9},
              {-8, 11, 1},
              { 0, -3, 4}}};
    matrix n{{{ 4,  6, 9},
              {-8, 11, 1},
              { 0, -3, 4}}};
    matrix o{{{ 1,  2, 3},
              {-8, 11, 1},
              { 0, -3, 4}}};
    ASSERT_TRUE(m == n);
    ASSERT_FALSE(m == o);
    ASSERT_TRUE(m != o);
    ASSERT_TRUE(n == m);
    ASSERT_TRUE(n != o);
    ASSERT_FALSE(n == o);
}

TEST(MatrixTest, Indexing) {
    using namespace linalg;
    const matrix m{ {{ 1, 2, 3},
                     { 4, 5, 6},
                     { 7, 8, 9}} };
    matrix n(m);
    ASSERT_THROW(m.at(-1, -1),basal::exception);
    ASSERT_THROW(m.at(0, 0), basal::exception);
    // The at() notation is 1-based
    ASSERT_DOUBLE_EQ(1.0, m.at(1,1));
    ASSERT_DOUBLE_EQ(6.0, m(1, 2));
    ASSERT_DOUBLE_EQ(7.0, m.index(6));
    ASSERT_DOUBLE_EQ(8.0, m[2][1]);
    ASSERT_DOUBLE_EQ(9.0, m.index(2,2));
    // The at() notation is 1-based
    ASSERT_DOUBLE_EQ(1.0, n.at(1,1));
    ASSERT_DOUBLE_EQ(6.0, n(1, 2));
    ASSERT_DOUBLE_EQ(7.0, n.index(6));
    ASSERT_DOUBLE_EQ(8.0, n[2][1]);
    ASSERT_DOUBLE_EQ(9.0, n.index(2,2));

    n.zero();
    ASSERT_DOUBLE_EQ(0.0, n[2][2]);
}

TEST(MatrixTest, StaticMethods) {
    using namespace linalg;
    matrix mZ = matrix::zeros(3, 3);
    matrix mI = matrix::identity(3, 3);
    for (size_t j = 0; j < mZ.rows; j++) {
        for (size_t i = 0; i < mZ.cols; i++) {
            ASSERT_EQ(0, mZ[j][i]);
        }
    }
    for (size_t j = 0; j < mI.rows; j++) {
        for (size_t i = 0; i < mI.cols; i++) {
            if (j == i) {
                ASSERT_EQ(1, mI[j][i]);
            } else {
                ASSERT_EQ(0, mI[j][i]);
            }
        }
    }
    ASSERT_TRUE(mZ != mI);
    ASSERT_FALSE(mZ == mI);
}

TEST(MatrixTest, ClassOperators) {
    using namespace linalg;
    using namespace linalg::operators;
    matrix A{ {{1,2},
               {3,4}} };
    matrix A2{{{2,2},
               {2,2}} };
    A += A2;
    matrix B{ {{3,4},
               {5,6}} };
    ASSERT_MATRIX_EQ(B, A);
    B -= A2;
    matrix C{ {{1,2},
               {3,4}} };
    ASSERT_MATRIX_EQ(C, B);
    C *= 2.0;
    matrix D{ {{2,4},
               {6,8}} };
    ASSERT_MATRIX_EQ(D, C);
    D /= 2.0;
    matrix E{ {{1,2},
               {3,4}} };
    ASSERT_MATRIX_EQ(E, D);
    B *= A2;
    matrix F{ {{6, 6}, {14, 14}} };
    ASSERT_MATRIX_EQ(F, B);
    matrix G{ {{1,2},
               {3,4}} };
    E /= G;
    matrix I = matrix::identity(2,2);
    ASSERT_MATRIX_EQ(I, E);
}

TEST(MatrixTest, NamespaceOperators) {
    using namespace linalg;
    using namespace linalg::operators;
    matrix A{ {{1,2},
               {3,4}} };
    matrix B{ {{5,6},
               {7,8}} };
    matrix C{ {{6,8},
               {10,12} }};
    matrix ApB = A + B;
    ASSERT_MATRIX_EQ(C, ApB);
    matrix D{ {{4,4},
               {4,4}} };
    matrix B_A = B - A;
    ASSERT_MATRIX_EQ(D, B_A);
    matrix E{ {{19, 22},
               {43, 50}}};
    matrix AB = A * B;
    ASSERT_MATRIX_EQ(E, AB);
    matrix Bi{ {{-4,3},{7/2,-5/2}} };
    matrix F{ {{3,-2},{2,-1}} };
    matrix AdB = A / B;
    ASSERT_MATRIX_EQ(F, AdB);
    matrix G{ {{15,18},{21,24}} };
    matrix Bm3 = B * 3.0;
    matrix Bd3 = Bm3 / 3.0;
    ASSERT_MATRIX_EQ(G, Bm3);
    ASSERT_MATRIX_EQ(B, Bd3);
}

TEST(MatrixTest, Inverse) {
    using namespace linalg;
    double h[2][2] = {{1, 2}, {3, 4}};
    double hi[2][2] = {{-2, 1}, {1.5, -0.5}};
    matrix H(h), Hi(hi);
    ASSERT_TRUE(H.inverse() == Hi);
}

TEST(MatrixTest, Invertible) {
    using namespace linalg;
    matrix A{{{1, 0, 1},
              {1, 1, 0},
              {0, 1, 1}}};
    ASSERT_TRUE(A.invertible());
    matrix D{{{2},{1}}};
    ASSERT_THROW(D.inverse(), basal::exception);
}

TEST(MatrixTest, Transpose) {
    using namespace linalg;
    matrix m{{{ 4,  6, 9},
              {-8, 11, 1},
              { 0, -3, 4}}};
    matrix mT{{{4, -8, 0},
               {6, 11, -3},
               {9, 1, 4}}};
    matrix mI = matrix::identity(3,3);
    ASSERT_TRUE(m != mT);
    ASSERT_FALSE(m == mT);
    ASSERT_TRUE(m.T() == mT);
    ASSERT_TRUE(mT == (m ^ letters::T));
    ASSERT_TRUE(mI == mI.T());
    ASSERT_TRUE(m == m.T().T());
}

TEST(MatrixTest, TransposeRules) {
    using namespace linalg;
    matrix A{{{1, 2, 3},
              {0, -4, 1},
              {0, 3, -1}}};
    matrix B = matrix::random(3, 3, 1.0, 10.0);
    matrix C{{{1, 4, 7},
              {3, 0, 5},
              {-1, 9, 11}}};
    double r = iso::tau / 2;
    // A = A^T^T
    ASSERT_TRUE(A == A.T().T());
    // (A+B)^T == A^T + B^T
    ASSERT_TRUE((A + B).T() == A.T() + B.T());
    // (rA)^T == r(A^T)
    ASSERT_TRUE((r * A).T() == r * (A.T()));
    // AB^T = B^T*A^T
    ASSERT_TRUE((A * B).T() == B.T() * A.T());
    // A^-1^T == A^T^-1
    ASSERT_TRUE(A.inverse().T() == A.T().inverse());
    {
        letters t = letters::T;
        // A^-1^T == A^T^-1
        matrix Ai = A ^ -1;
        matrix Ait = Ai ^ t;
        matrix At = A ^ t;
        matrix Ati = At ^ -1;
        ASSERT_TRUE(Ait == Ati);
    }
    // A^-1^T == A^T^-1
    ASSERT_TRUE(inv(A).T() == inv(A.T()));
    ASSERT_TRUE((A * (A ^ T)).symmetric());
    ASSERT_TRUE((A + (A ^ T)).symmetric());
    ASSERT_TRUE((A - (A ^ T)).skew_symmetric());
}

TEST(MatrixTest, SingleAssignment) {
    using namespace linalg;
    matrix mZ = matrix::zeros(3, 3);
    matrix ones(3, 3); // empty
    ones = 1.0; // Single Assignment to all elements
    ASSERT_TRUE(mZ == (ones - ones));
}

TEST(MatrixTest, Determinants) {
    using namespace linalg;
    matrix A{{{1, 2, 3},
              {0, -4, 1},
              {0, 3, -1}}};
    matrix C{{{1, 4, 7},
              {3, 0, 5},
              {-1, 9, 11}}};
    matrix D{{{8}}};
    matrix zD = D.inverse();
    ASSERT_DOUBLE_EQ(8.0, D.determinant());
    ASSERT_DOUBLE_EQ(0.125, zD[0][0]);
    matrix mZ = matrix::zeros(3, 3);
    // det(A) == 1
    ASSERT_DOUBLE_EQ(1.0, det(A));
    ASSERT_DOUBLE_EQ(1.0, A.determinant());
    // det(zeroes) == 0
    ASSERT_DOUBLE_EQ(0.0, det(mZ));
    ASSERT_DOUBLE_EQ(0.0, mZ.determinant());
    // determinents are communicative
    ASSERT_TRUE(det(A * C) == det(A) * det(C));
}

TEST(MatrixTest, Cofactors) {
    using namespace linalg;
    matrix C{{{1, 4, 7},
              {3, 0, 5},
              {-1, 9, 11}}};
    ASSERT_DOUBLE_EQ(13.0, C.minor(1, 2));
    ASSERT_DOUBLE_EQ(-13.0, C.cofactor(1, 2));
}

TEST(MatrixTest, NonSingularRules) {
    using namespace linalg;
    matrix A{{{1, 2, 3},
              {0, -4, 1},
              {0, 3, -1}}};
    matrix mI = matrix::identity(3, 3);
    ASSERT_FALSE(A.singular());
    ASSERT_FALSE(A.degenerate());
    ASSERT_TRUE(A * A.inverse() == mI);
    ASSERT_TRUE(A.inverse() * A == mI);
    ASSERT_TRUE(A / A == mI);
    ASSERT_TRUE(A.inverse().inverse() == A);
    ASSERT_TRUE(A * (A ^ -1) == mI);
    ASSERT_TRUE((A ^ -2) == A);
    ASSERT_TRUE((A ^ 0) == mI);
}

TEST(MatrixTest, Adjugate) {
    using namespace linalg;
    matrix E{{{-3, 2, -5},
              {-1, 0, -2},
              {3, -4, 1}}};
    matrix _E{{{-8, 18, -4},
               {-5, 12, -1},
               {4, -6, 2}}};
    ASSERT_TRUE(E.adjugate() == _E);
}

TEST(MatrixTest, SingularRules) {
    using namespace linalg;
    double f[2][2] = {{2, 6}, {1, 3}};
    matrix F(f);
    ASSERT_TRUE(F.singular());
    ASSERT_TRUE(F.degenerate());
    ASSERT_FALSE(F.invertible());
    ASSERT_THROW(F.inverse(), basal::exception);
}

TEST(MatrixTest, Orthgonal) {
    using namespace linalg;
    matrix I = matrix::identity(3,3);
    ASSERT_TRUE(I.orthagonal());
}

TEST(MatrixTest, FileWriting) {
    using namespace linalg;
    std::string path = "./m.bin";
    matrix m{{{ 4,  6, 9},
              {-8, 11, 1},
              { 0, -3, 4}}};
    m.print("old m ");
    ASSERT_TRUE(m.to_file(path));
    matrix n = matrix::from_file(path);
    n.print("new m? ");
    ASSERT_TRUE(m == n);
}

TEST(MatrixTest, PairwiseMultiply) {
    using namespace linalg;
    matrix A{ {{1,3,5}} };
    matrix B{ {{2},{4},{6}} };
    matrix C = pairwise::multiply(A, B);
    matrix D{ {{2, 6, 10}, {4, 12, 20}, {6, 18, 30}} };
    ASSERT_TRUE(C == D);
}

TEST(MatrixTest, PairwiseMultiplyThrow) {
    using namespace linalg;
    matrix A{ {{1,3,5}} };
    matrix B{ {{2,4},
            {4,6}} };
    ASSERT_THROW(pairwise::multiply(A, B), basal::exception);
}

TEST(MatrixTest, HammardOperation) {
    using namespace linalg;
    matrix A{ {{1},{3},{5}} };
    matrix B{ {{7},{9},{6}} };
    matrix C = hadamard(A, B);
    matrix D{ {{7},{27},{30}} };
    ASSERT_TRUE(C == D);
}

TEST(MatrixTest, RuleOfSarrus) {
    using namespace linalg;
    matrix A{ {{1, 2, 3},
                {4, 5, 6},
                {7, 8, 9}} };
    matrix B{ {{1, 2, 3, 1, 2},
                {4, 5, 6, 4, 5},
                {7, 8, 9, 7, 8}} };
    ASSERT_TRUE(A.rule_of_sarrus() == B);
    ASSERT_THROW(B.rule_of_sarrus(), basal::exception);
}

TEST(MatrixTest, NullspaceIdentity) {
    using namespace linalg;
    matrix I = matrix::identity(3, 3);
    ASSERT_THROW(I.nullspace(), basal::exception);
}

TEST(MatrixTest, NullspaceMatrix) {
    using namespace linalg;
    matrix A{ {{ 1,  0, -3,  0,  2, -8},
               { 0,  1,  5,  0, -1,  4},
               { 0,  0,  0,  1,  7, -9},
               { 0,  0,  0,  0,  0,  0}} };
    auto pc = A.pivots();
    ASSERT_TRUE(pc.size() == 3);
    auto fv = A.frees();
    ASSERT_TRUE(fv.size() == 3);
    matrix B = A.nullspace();
    matrix C{ {{ 3, -2,  8},
               {-5,  1, -4},
               { 1,  0,  0},
               { 0, -7,  9},
               { 0,  1,  0},
               { 0,  0,  1}} };
    matrix D = A.basis();
    matrix E{ {{1, 0, 0},
               {0, 1, 0},
               {0, 0, 1},
               {0, 0, 0}} };
    ASSERT_TRUE(B == C);
    ASSERT_TRUE(D == E);
}

TEST(MatrixTest, Joinery) {
    using namespace linalg;
    double f[2][2] = {{2, 6}, {1, 3}};
    matrix F(f);
    double g[2][4] = {{2, 6, 1, 2}, {1, 3, 3, 4}};
    matrix G(2, 4, g[0]);
    double h[2][2] = {{1, 2}, {3, 4}};
    double hi[2][2] = {{-2, 1}, {1.5, -0.5}};
    matrix H(h), Hi(hi);
    double j[4][2] = {{2, 6}, {1, 3}, {1, 2}, {3, 4}};
    matrix J(4, 2, j[0]);
    ASSERT_TRUE(G == (F | H));
    ASSERT_THROW((F | H).inverse(), basal::exception);
    ASSERT_TRUE(J == (F || H));
    ASSERT_THROW((F || H).inverse(), basal::exception);
}

TEST(MatrixTest, AssignIntoRowCol) {
    using namespace linalg;
    matrix A{ {{1, 2, 3},
               {4, 5, 6},
               {7, 8, 9}} };
    matrix A_c1{ {{2}, {5}, {8}} };
    matrix a_c1 = A.col(1);
    ASSERT_TRUE(a_c1 == A_c1);
    matrix A_r1{ {{4, 5, 6}} };
    matrix a_r1 = A.row(1);
    ASSERT_TRUE(a_r1 == A_r1);
    matrix B = matrix::zeros(2, 2);
    matrix C{ {{0, 0, 3},
               {0, 0, 6},
               {7, 8, 9}} };
    matrix D = A;
    B.assignInto(A, 0, 0);
    ASSERT_TRUE(C == A);
    matrix E{ {{1, 2, 3},
               {4, 0, 0},
               {7, 0, 0}} };
    B.assignInto(D, 1, 1);
    ASSERT_TRUE(D == E);
}

TEST(MatrixTest, Nullspace) {
    using namespace linalg;
    matrix A{{{2, 3, 5},
                {-4, 2, 3}}};
    matrix Arref{{{1, 0, 1.0 / 16},
                    {0, 1, 13.0 / 8}}};
    ASSERT_TRUE(A.rref() == Arref);
    matrix B{{{1, 2, 3, 4, 5},
                {2, 5, 8, 11, 14},
                {1, 3, 5, 8, 11},
                {4, 10, 16, 23, 30}}};
    matrix Brref{{{1, 0, -1, 0, 1},
                    {0, 1, 2, 0, -2},
                    {0, 0, 0, 1, 2},
                    {0, 0, 0, 0, 0}}};
    ASSERT_MATRIX_EQ(Brref, B.rref());
    matrix C = B.nullspace();
    //print_this(C);
    matrix N{{{1, -1}, {-2, 2}, {1, 0}, {0, -2}, {0, 1}}};
    ASSERT_MATRIX_EQ(N, C);
}

TEST(MatrixTest, Pivots) {
    using namespace linalg;
    matrix A{{{1, 3, 1, 9},
                {1, 1, -1, 1},
                {3, 11, 5, 35}}};
    //print_this(A);
    matrix B{{{1, 0, -2, -3},
                {0, 1, 1, 4},
                {0, 0, 0, 0}}};
    //print_this(B);
    ASSERT_MATRIX_EQ(B, A.escheloned().reduced());
    std::vector<size_t> pivots = B.pivots();
    for (auto& p : pivots) {
        std::cout << p << std::endl;
    }
    ASSERT_EQ(2, pivots.size());
    ASSERT_EQ(0, pivots[0]);
    ASSERT_EQ(1, pivots[1]);
    std::vector<size_t> freev = B.frees();
    ASSERT_EQ(2, freev.size());
    ASSERT_EQ(2, freev[0]);
    ASSERT_EQ(3, freev[1]);
}

TEST(MatrixTest, EigenVectors) {
    using namespace linalg;
    // find eigenvector of matrix then test
    matrix A{ {{3, 2, 2},
               {2, 3,-2}} };
    matrix AAT = A*A.T();
    matrix f{ {{25},{9}} };
    ASSERT_MATRIX_EQ(f, AAT.eigenvalues());
}

TEST(MatrixTest, Rank) {
    using namespace linalg;
    matrix A{ {{ 1,  2,  1},
               {-2, -3,  1},
               { 3,  5,  0}} };
    ASSERT_EQ(2, A.rank());
}

TEST(MatrixTest, Triangluar) {
    using namespace linalg;
    matrix mI = matrix::identity(5, 5);
    matrix A{ {{1, 2, 3},
               {0, -4, 1},
               {0, 3, -1}} };
    matrix D{ {{1, 0, 0},
               {0, 9, 0},
               {0, 0, 0}} };
    ASSERT_TRUE(D.diagonal());
    ASSERT_TRUE(mI.diagonal());
    ASSERT_FALSE(A.diagonal());
    ASSERT_TRUE(D.triangular());
    matrix E{ {{7, 0, 0, 0},
               {9, 1, 0, 0},
               {-1, 2, 4, 0},
               {5, 6, 1, 1}} };
    ASSERT_TRUE(E.lower_triangular());
    ASSERT_TRUE(E.T().upper_triangular());
    ASSERT_TRUE(E.inverse().lower_triangular());
    ASSERT_TRUE(E.T().inverse().upper_triangular());
}

TEST(MatrixTest, DISABLED_SVD) {
    /*! This uses data from:
    * <ul>
    * <li>Elementary Linear Algebra by Howard Anton and Chris Rorres, aka. ELA</li>
    * <li>Wolfram Math World</li>
    * <li>Wikipedia</li>
    */
    using namespace linalg;
    matrix A{ {{3,2,2},{2,3,-2}} };
    matrix AAT = A * A.T();
    matrix ATA = A.T() * A;
    matrix e1 = AAT.eigenvalues();
    matrix e2 = ATA.eigenvalues();
    // FIXME (Gtest) SVD tests are incomplete. Put in actual data so it can be verified.
    SUCCEED();
}

TEST(MatrixTest, PLU) {
    using namespace linalg;
    matrix M{ {{ 0, 3, 0, 0, 9},
               { 0, 0, 0, 0, 0},
               { 0, 0, 4,-3, 1},
               { 0, 0, 0, 0, 0},
               { 0, 0, 0,-5, 8},
               { 0, 7, 3, 0, 2},
               { 0, 0, 0, 0, 0}} };
    matrix N{ {{ 0, 2, 8,-7},
               { 2,-2, 4, 0},
               {-3, 4,-2,-5}} };
    matrix N_rre{ {{ 1, 0, 6, 0},
                   { 0, 1, 4, 0},
                   { 0, 0, 0, 1}} };
    matrix O{ {{3,2,2,3,1},
               {6,4,4,6,2},
               {9,6,6,9,1}} };
    matrix A{ {{1, 2, 3, 4},
               {5, 6, 7, 8},
               {1,-1, 2, 3},
               {2, 1, 1, 2}} };
    A.print("A");
    matrix P(4, 4);
    matrix L(4, 4);
    matrix U(4, 4);
    A.PLU(P, L, U);
    P.print("P");
    L.print("L");
    U.print("U");
    ASSERT_MATRIX_EQ(A, (L*U));
    ASSERT_MATRIX_EQ((P*A), (L*U));
}