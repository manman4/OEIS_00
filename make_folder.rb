require 'fileutils'

##################################################################
#
# 1. b_fileの名前に基づきフォルダ作成、b_file格納
#
#    ex. b012345.txt があるとき、012345/b012345.txt1に移動
#
# 2. フォルダに実行ファイル（rb, py）格納
#
#    ex. 012345.rb または 012345.py があるとき
#    012345/b012345_01.rbに移動　（ファイル名に_01を追加）
#
##################################################################


dir_name = "036"
dir = Dir.open(dir_name)

dir.each{|file_name|
  if file_name[-4..-1] == ".txt" && file_name[0] == "b"
    b_file_name = file_name[1..-5]
    # 6桁＋α
    if b_file_name.size == 6
      # p b_file_name
      path = dir_name + "/" + b_file_name
      # フォルダがなければ作成
      FileUtils.mkdir_p(path) unless FileTest.exist?(path)

      # b_fileの移動
      # 同名のファイルがなければ、フォルダの中に移動
      if !(File.exists?(path + "/" + file_name))
        FileUtils.mv(dir_name + "/" + file_name, path + "/" + file_name)
      end
    end
  end
}

dir.each{|file_name|
  if file_name[-3..-1] == ".rb" || file_name[-3..-1] == ".py"
    executable_file_name = file_name[0..-4]
    extension = file_name[-3..-1]
    # 6桁＋α
    if executable_file_name.size == 6
      path = dir_name + "/" + executable_file_name
      # フォルダがなければ作成
      # (通常b_flleが作成されており、フォルダが上記プロセスでできているはずだが、
      #  b_fileがないこともたまにあるのでこの処理を行う)
      FileUtils.mkdir_p(path) unless FileTest.exist?(path)

      # executable_fileの移動
      # 同名のファイル(_01.*)がなければ、フォルダの中に移動
      if !(File.exists?(path + "/" + executable_file_name + "_01" + extension))
        FileUtils.mv(dir_name + "/" + file_name, path + "/" + executable_file_name + "_01" + extension)
      end
    end
  end
}