# Julia script for testing simple example

include("example.jl")
using Example

# Call our gcd() function

x = 42 % Int32
y = 105 % Int32
g = Example.gcd(x, y)
println("The gcd of $x and $y is $g")

# Manipulate the Foo global variable

# Output its current value
println("Foo = $(Example.Foo())")

# Change its value
Example.Foo!(3.1415926)

# See if the change took effect
println("Foo = $(Example.Foo())")

