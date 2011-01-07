require 'rubygems'
require 'bundler'
Bundler.setup(:development)

$LOAD_PATH.unshift File.expand_path('../../ext', __FILE__)

require 'rspec'
require 'Stata'

