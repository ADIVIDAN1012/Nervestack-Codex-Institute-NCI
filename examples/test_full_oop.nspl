< Test Property Access and Method Calls >

blueprint Counter {
    shard count
    
    spec increment(own) {
        own.count = own.count + 1
    }
    
    spec show_count(own) {
        show("Count:")
        show(own.count)
    }
}

spec main() {
    show("=== Testing Full OOP ===")
    
    show("1. Creating Counter...")
    firm c = spawn Counter()
    show("Counter created!")
    
    show("2. Setting property...")
    c.count = 0
    show("Property set!")
    
    show("3. Calling method...")
    funcall c.show_count()
    
    show("4. Incrementing...")
    funcall c.increment()
    
    show("5. Checking new value...")
    funcall c.show_count()
    
    show("=== OOP Test Complete ===")
}

funcall main()
