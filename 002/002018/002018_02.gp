\\ a(n) = (n-1)! * n! * [x^(n-1)] 1/(1-x)^(3/2) * exp(x/2) for n > 0.
a(n) = my(x='x+O('x^(n+1))); if(n==0, 1, (n-1)! * n! * polcoef( 1/(1-x)^(3/2) * exp(x/2), n-1));
for(n=0, 20, print1(a(n),", "))