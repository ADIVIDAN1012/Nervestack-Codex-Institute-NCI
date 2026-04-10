< Test Full OOP - Methods >

blueprint Counter {
    shard count
    
    spec show_value(own) {
        show("Count is:")
        show(own.count)
    }
}

spec main() {
    show("=== Full OOP Test ===")
    
    show("1. Creating Counter...")
    firm c = spawn Counter()
    
    show("2. Setting property...")
    c.count = 5
    
    show("3. Calling method...")
    funcall c.show_value()
    
    show("=== Test Complete ===")
}

funcall main()
