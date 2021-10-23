require 'find'

# Axxxyyyの場合./xxxを指定。指定しない場合は全部
path = ARGV[0] ? ARGV[0] : '.'
# .txtはgitignoreしているので.rbファイルに保存
cnt = 0
File.open("squence_list.rb", mode = "w"){|f|
  f.write("#{Time.now.strftime("%Y-%m-%d %H:%M:%S")} 現在\n")
  Find.find(path){|item|
    # ./git とか省く
    if  /[0-9]{3}/.match?(File.dirname(item)[2..4])
      # ディレクトリのみ対象
      if File.ftype(item) == "directory"
        cnt += 1
        base = File.basename(item)
        f.write("#{cnt} #{base}\n")
      end
    end
  }
}
