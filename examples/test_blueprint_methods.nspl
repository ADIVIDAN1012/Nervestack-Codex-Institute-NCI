< Test Blueprint with Methods >

blueprint Counter {
    shard count
    
    spec increment(own) {
        own.count = own.count + 1
        show("Incremented!")
    }
    
    spec get_value(own) {
        show("Current count:")
        show(own.count)
    }
}

spec main() {
    show("Creating Counter blueprint...")
    firm counter = spawn Counter()
    
    show("Setting initial value...")
    counter.count = 0
    
    show("Testing methods...")
    funcall counter.get_value()
    funcall counter.increment()
    funcall counter.get_value()
    
    show("Blueprint methods test complete!")
}

funcall main()
