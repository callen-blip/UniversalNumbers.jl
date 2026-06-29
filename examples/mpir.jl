# Mixed-precision iterative refinement (MPIR) for a sparse linear system A x = b
#
# Requires MatrixDepot.jl (one-time install):
#   julia --project=examples -e 'using Pkg; Pkg.instantiate()'
#
# Run from the project root:
#   julia --project=examples examples/mpir.jl
#
# Three precisions (Carson & Higham, SIAM 2018):
#   LP  factorization precision (low)   -- LU computed here
#   WP  working precision               -- A, b, and the solution x live here
#   HP  residual precision (high)       -- r = b - A x computed here
#
# IR drives a cheap low-precision factorization up to WORKING-precision (WP)
# backward accuracy.  It therefore stops when the *relative* residual reaches
# eps(WP): ||r||/||b|| ~ eps(WP).  For arc130 with WP = Posit{32,2} that floor
# is ||r|| ~ 2e-4 (since ||b|| ~ 2e6) -- this is convergence, not a stall.
# To push the residual lower, raise WP (e.g. Posit{64,2}); the solution can
# only be as accurate as the precision it is stored in.

using UniversalNumbers
using LinearAlgebra
using SparseArrays
using MatrixDepot

LP = Posit{16,2}   # low precision  (factorization)
WP = Posit{32,2}   # working precision (A, b, x)
HP = Posit{64,2}   # high precision (residual)

# ID = 6 is arc130 from the Harwell-Boeing collection
A = SparseMatrixCSC{WP, Int64}(matrixdepot(sp(6)))
b = A * ones(WP, size(A, 1))

# Factor A in low precision, then cast the factors to WP for the solves
L, U = UniversalNumbers.LU.lu(LP.(A))
Lw = WP.(L)
Uw = WP.(U)

# Initial solve in working precision: x0 = U \ (L \ b)
x = Uw \ (Lw \ b)

# Stop at working-precision backward error, or when progress stalls
tol      = Float64(eps(WP)) * norm(Float64.(b))   # relative -> absolute target
maxiters = 50
prev     = Inf

for k in 1:maxiters
    # residual in high precision, then back to working precision
    r  = HP.(b) - HP.(A) * HP.(x)
    rn = norm(Float64.(r))
    println("iter $k: ||r|| = $rn")

    (rn < tol || rn >= prev) && break    # converged, or no longer improving
    global prev = rn

    # correction solve reusing the LU factors, then update x in working precision
    c = Uw \ (Lw \ WP.(r))
    global x = x + WP.(c)
end

println("final ||r||            = ", norm(Float64.(b) - Float64.(A) * Float64.(x)))
println("target (eps(WP)*||b||) = ", tol)
