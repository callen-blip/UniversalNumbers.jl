using UniversalNumbers
using LinearAlgebra
using SparseArrays
using MatrixDepot

T = Posit{16,2}

# ID = 6 is arc130 from the Harwell-Boeing collection
# 130 x 130 with condition number 6e+10
# See: https://sparse.tamu.edu/HB/arc130
A = SparseMatrixCSC{T, Int64}(matrixdepot(sp(6)))
# or  
# A = T.(matrixdepot(sp(6)))

# Another example of a (random) sparse matrix (without MatrixDepot).
B = sparse(rand(T, 10,10))
sparse([1, 2, 3], [1, 3, 4, 7], [T(1), T(2), T(3), T(4)], 10, 10) # sparse diagonal matrix


# Random Sparse (specifying the density of nonzeros as 40%).
C = sprand(10, 10, 0.4) + 5I

# and another (converts a dense matrix to sparse)
D = sparse(T.([4.0 1.0 0.0; 1.0 5.0 2.0; 0.0 2.0 6.0]))

# using SparseMartixCSC constructor directly
E = SparseMatrixCSC{T, Int64}([4.0 1.0 0.0; 1.0 5.0 2.0; 0.0 2.0 6.0])

# -------------------------------------------------------
# Notes on the condition number of sparse matrices.
# Sparse matrices with type T have to be converted to 
#   Float64 to compute the condition number.
cond(Float64.(A),1) # Sparse matrices have to use 1-norm (or Inf-norm)
cond(Matrix(Float64.(A))) # 6e+10

# -------------------------------------------------------
# Special sparse matrix types
Diagonal(T.(diag(A))) # Diagonal of A
LowerTriangular(T.(tril(A))) # Lower triangular part of A
UpperTriangular(T.(triu(A))) # Upper triangular part of A
