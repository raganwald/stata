require 'spec_helper'
require 'tempfile'

describe 'Stata.read' do
  it "raises ArgumentError if no filename is provided" do
    expect { Stata.read }.to raise_error(ArgumentError)
  end

  it "raises ArgumentError if filename is not a string" do
    expect { Stata.read(nil) }.to raise_error(ArgumentError)
    expect { Stata.read(1) }.to raise_error(ArgumentError)
  end

  it "raises RuntimeError if filename does not exist" do
    expect { Stata.read("nonexistent_file.dta") }.to raise_error(RuntimeError)
  end

  it "raises RuntimeError if file is not a valid DTA file" do
    Tempfile.open(['read_spec', '.dta']) do |f|
      f.puts "blah"
      f.close
      expect { Stata.read(f.path) }.to raise_error(RuntimeError)
    end
  end
end

