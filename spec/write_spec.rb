require 'spec_helper'
require 'tempfile'

describe 'Stata.write' do
  keys = ["data_label", "time_stamp", "nvar", "nobs", "data", "fields", "value_labels"]

  before :each do
    @s = {
      "data_label" => "",
      "time_stamp" => "",
      "nvar" => 2,
      "nobs" => 2,
      "data" => [[1, 2.0], [3, 4.0]],
      "fields" => [
        {"name" => "foo", "type" => 252, "format" => "%-8.0g", "sort" => 1, "variable_label" => "Foo", "value_label" => ""},
        {"name" => "bar", "type" => 254, "format" => "%-9.0g", "sort" => 0, "variable_label" => "Bar", "value_label" => "bar"}
      ],
      "value_labels" => [{"name" => "bar", "table" => [[2, "Two"], [4, "Four"]]}]
    }

    @file = Tempfile.new(['write_spec', '.dta'])
  end

  after :each do
    @file.close
  end

  keys.each do |key|
    it "raises ArgumentError if #{key.inspect} is missing" do
      @s.delete(key)
      expect { Stata.write(@file.path, @s) }.to raise_error(ArgumentError)
    end

    it "raises ArgumentError if #{key.inspect} is not of the right type" do
      @s[key] = Object.new
      expect { Stata.write(@file.path, @s) }.to raise_error(ArgumentError)
    end
  end

  it "produces a file that, when read by Stata.read, is identical to the original data structure" do
    Stata.write(@file.path, @s)

    s = Stata.read(@file.path)

    s.delete("file_name")

    s["fields"].each do |f|
      f.delete("id")
    end

    s.should == @s
  end
end
