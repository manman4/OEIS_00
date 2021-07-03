require 'prime'

# mŸˆÈ‰º‚ğæ‚èo‚·(•ÏŒ`”Å)
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    ary[i]          += f_ary[i]
    ary[i + s2 - 1] += f_ary[i] * b_ary[s2 - 1]
  }
  ary[0..m]
end

def A(n)
  ary = [1]
  (1..n).each{|i|
    if i.prime?
      b_ary = Array.new(i + 1, 0)
      b_ary[0], b_ary[-1] = 1, 1
      ary = mul(ary, b_ary, n)
    end
  }
  ary
end

n = 10000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
