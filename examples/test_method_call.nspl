< Test Method Call - Simplest >

blueprint Greeter {
    spec say_hello(own) {
        show("Hello from method!")
    }
}

spec main() {
    show("Creating instance...")
    firm g = spawn Greeter()
    
    show("Calling method...")
    funcall g.say_hello()
    
    show("Method call complete!")
}

funcall main()
