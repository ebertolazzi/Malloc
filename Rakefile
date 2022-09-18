require_relative "./cmake_utils/Rakefile_common.rb"

desc "setup Eigen3, FMT and ZSTR"
task :install_3rd do
  FileUtils.cd "ThirdParties"
    sh "rake install"
  FileUtils.cd ".."
end

task :build_common, [:bits] => :install_3rd do |t, args|
  args.with_defaults( :bits => "x64" )

  puts "UTILS build (osx/linux/mingw/windows)".green

  FileUtils.rm_rf   'lib'
  FileUtils.rm_rf   'build'
  FileUtils.mkdir_p 'build'
  FileUtils.cd      'build'

  puts "run CMAKE for UTILS".yellow
  sh "cmake -G Ninja -DBITS:VAR=#{args.bits} " + cmd_cmake_build() + ' ..'
  puts "compile with CMAKE for UTILS".yellow
  if COMPILE_DEBUG then
    sh 'cmake --build . --config Debug --target install '+PARALLEL
  else
    sh 'cmake --build . --config Release --target install '+PARALLEL
  end

  FileUtils.cd '..'
end

desc 'compile for OSX'
task :build_osx => :build_common do end

desc 'compile for LINUX'
task :build_linux => :build_common do end

desc 'compile for MINGW'
task :build_mingw => :build_common do end

desc 'compile for WINDOWS'
task :build_win do
  # check architecture
  case `where cl.exe`.chop
  when /x64\\cl\.exe/
    VS_ARCH = 'x64'
  when /amd64\\cl\.exe/
    VS_ARCH = 'x64'
  when /bin\\cl\.exe/
    VS_ARCH = 'x86'
  else
    raise RuntimeError, "Cannot determine architecture for Visual Studio".red
  end
  Rake::Task[:build].invoke(VS_ARCH)
end

task :clean do
  FileUtils.rm_rf 'lib'
end

desc "clean for OSX"
task :clean_osx => :clean

desc "clean for LINUX"
task :clean_linux => :clean

desc "clean for MINGW"
task :clean_mingw => :clean

desc "clean for WINDOWS"
task :clean_win => :clean

task :cppcheck do
  FileUtils.rm_rf   'lib'
  FileUtils.rm_rf   'build'
  FileUtils.mkdir_p 'build'
  FileUtils.cd      'build'
  sh 'cmake -DCMAKE_EXPORT_COMPILE_COMMAND=ON ..'
  sh 'cppcheck --project=compile_commands.json'
end

desc 'pack for OSX/LINUX/MINGW/WINDOWS'
task :cpack do
  FileUtils.cd "build"
  puts "run CPACK for ROOTS".yellow
  sh 'cpack -C CPackConfig.cmake'
  sh 'cpack -C CPackSourceConfig.cmake'
  FileUtils.cd ".."
end
