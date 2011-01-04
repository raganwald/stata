# encoding: utf-8

Gem::Specification.new do |s|
  s.name     = 'stata'
  s.version  = '0.1.0'
  s.authors  = ['Kevin Branigan']
  s.summary  = 'Read and write support for the Stata binary format'
  s.homepage = 'http://github.com/unspace/stata'
  s.license  = 'MIT'

  s.extensions    = 'ext/extconf.rb'
  s.files         = Dir.glob('ext/**/*') + %w(LICENSE README)
  s.require_paths = 'ext'
end

