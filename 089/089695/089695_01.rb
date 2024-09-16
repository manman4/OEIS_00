require 'prime'

def split_number(number_str)
  # ベースケース: 1桁ならそのまま返す
  return [[number_str]] if number_str.size == 1

  result = []
  
  # 再帰的に、すべての分割パターンを探索
  (1..number_str.size - 1).each{|i|
    left_part = number_str[0..i - 1]
    right_part = number_str[i..-1]
    
    split_number(right_part).each{|split_right|
      result << [left_part] + split_right
    }
  }

  # 自分自身の数字全体を1つの塊として追加
  result << [number_str]
  
  result
end

b=[]
Prime.take(10000).each{|i|

  # 数字を文字列に変換
  number_str = i.to_s
  flag = true
  split_number(number_str).each{|i|
    num = i.inject(0){|sum, j| sum += j.to_i}
    if !num.prime?
      flag = false
      break
    end
  }
  if flag
    b << i
    p i
  end
}

p b

