require 'bigdecimal/math'
include BigMath

def pi(n)
  m = (BigDecimal.new(n.to_s).sqrt(100)).to_i
  keys = (1..m).map{|i| n / i}
  keys += (1..keys[-1] - 1).to_a.reverse
  h = {}
  keys.each{|i| h[i] = i - 1}
  (2..m).each{|i|
    if h[i] > h[i - 1]
      hp = h[i - 1]
      i2 = i * i
      h[n] -= h[n / i] - hp
      keys[i..-1].each{|j|
        break if j < i2
        h[j] -= h[j / i] - hp
      }
    end
  }
  h[n]
end

puts '0 0'
(1..32).each{|i|
  print i
  print ' '
  puts pi(Math.exp(i).to_i)
}
