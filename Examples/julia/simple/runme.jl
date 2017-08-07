# Julia script for testing simple example

include("simple.jl")
using Simple

# Call our gcd() function

x = 42
y = 105
g = Simple.gcd(x, y)
println("The gcd of $x and $y is $g")

# Manipulate the Foo global variable

# Output its current value
println("Foo = $(Simple.Foo())")

# Change its value
Simple.Foo(3.1415926)

# See if the change took effect
println("Foo = $(Simple.Foo())")

