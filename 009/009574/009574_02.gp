my(N=30, x='x+O('x^N)); concat(0, Vec(serlaplace( x * (1+x/2) * exp(x) / (1+x) )))

\\ sinh(log(1+x))*exp(x)
my(N=30, x='x+O('x^N)); concat(0, Vec(serlaplace( sinh(log(1+x))*exp(x) )))
\\ 一致確認
my(N=30, x='x+O('x^N)); concat(0, Vec(serlaplace( sinh(log(1+x))*exp(x) - x * (1+x/2) * exp(x) / (1+x) )))