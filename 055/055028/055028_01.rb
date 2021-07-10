require 'prime'

def A(n)
  ary = [0, 0, 1]
  (3..n).each{|i|
    sq = Math.sqrt(i).to_i
    if i.prime? && i % 4 == 1
      s = 0
      (1..sq).each{|j|
        # i = q * q‚Æ‚È‚ç‚È‚¢‚Ì‚ÅAk > 0
        k = i - j * j
        s += 1 if Math.sqrt(k).to_i ** 2 == k
      }
      ary << s
    elsif sq ** 2 == i
      if sq.prime? && sq % 4 == 3
        ary << 1
      else
        ary << 0
      end
    else
      ary << 0
    end
  }
  ary[0..n]
end

n = 10000
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i] * 4
}
