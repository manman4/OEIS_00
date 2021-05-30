require 'OpenSSL'

def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
end

# a‚Í•ª”
def f(a, b)
  a.numerator * OpenSSL::BN.new(a.denominator.to_s).mod_inverse(b).to_i % b
end

n = 1000
b_ary = bernoulli(n)
(1..n).each{|i|
  print i
  print ' '
  puts f(-i * b_ary[i - 1], i)
}
