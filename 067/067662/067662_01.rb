require 'prime'

n = 500
s = 0
cnt = 0
while cnt < n
  s += 1
  if (s * s + 1).prime? && (s * s * s * s + 1).prime?
    cnt += 1
    print cnt
    print ' '
    puts s
  end
end
