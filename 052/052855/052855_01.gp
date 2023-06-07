seq(n) = {
    my(A=1); 
    for(i=1, n, 
        A=exp(sum(k=1, i, subst(A, x, x^k)*x^k/k/(1-x^k))+x*O(x^n))
    ); 
    Vec(1/prod(j=1, n, prod(k=0, n, (1-x^(j+k)+x*O(x^n))^polcoef(A, k) )))
};
seq(30)

