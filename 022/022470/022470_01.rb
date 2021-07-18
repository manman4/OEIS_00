def lookandsay(str)
  str.gsub(/(.)\1*/) {$1 + $&.length.to_s}
end

n = 100
num = "2"
(1..n).each{|i|
  break if num.to_s.size > 1000
  print i
  print ' '
  puts num
  num = lookandsay(num)
}
