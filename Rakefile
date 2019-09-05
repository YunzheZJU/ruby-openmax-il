require 'rake/extensiontask'
require 'rubygems/package_task'

##
# Rake::ExtensionTask comes from the rake-compiler and understands how to
# build and cross-compile extensions.
#
# See https://github.com/luislavena/rake-compiler for details

Rake::ExtensionTask.new 'ruby_openmax_il' do |ext|

  # This causes the shared object to be placed in lib/my_malloc/my_malloc.so
  #
  # It allows lib/my_malloc.rb to load different versions if you ship a
  # precompiled extension that supports multiple ruby versions.

  ext.lib_dir = 'lib/ruby_openmax_il'
end

s = Gem::Specification.new 'ruby_openmax_il', '1.0' do |s|
  s.summary = 'My OpenMax IL wrapper'
  s.authors = %w[yunzhe@zju.edu.cn]

  # this tells RubyGems to build an extension upon install

  s.extensions = %w[ext/ruby_openmax_il/extconf.rb]

  # naturally you must include the extension source in the gem

  s.files = %w[
    MIT-LICENSE
    Rakefile
    ext/ruby_openmax_il/extconf.rb
    ext/ruby_openmax_il/ruby_openmax_il.c
    lib/ruby_openmax_il.rb
  ]
end

# The package task builds the gem in pkg/my_malloc-1.0.gem so you can test
# installing it.

Gem::PackageTask.new s do end

task test: %w[compile] do
  ruby '-Ilib', '-rruby_openmax_il', 'test/test.rb'
end

task default: :test

