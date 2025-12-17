# Marshalling library
Currently Supports:
* JSON (currently encoder only - not sure where the decodder is)
* XML (encoder and decoder)

I actually regret doing it this way - for XML it worked fine but for JSON it was hell.. It is still a long way from supporting
proper JSON..  basic JSON is fine...
Callback based is just so much easier/simpler than API-based. However, it requires your classes to inherit interfaces.
Which was a dependency I wanted to avoid...

JSON is problematic because of the Array's can contain mixed types in any order - when consuming them in to C++ from a an API usage we either
limit us or each class has to implement a bunch of logic which is redundant...

# Build
Add the subdirectory to your project and link with the libraray.

# Testing
You need the test-runner (see: https://github.com/gnilk/testrunner) in order to run unit tests.

# Serialization
Serialization is easy enough to understand, you just need to implement `ISerialize` in your data class and it can be 
used to encode. You might want to add a specialization to the JSON encoder to support 'attributes' from XML. I will
probably do that - but it has not been done yet.

# Deserialization
There are basically two 'simple' ways to do this (without template magic) in C++. Either you have callbacks that your
data class needs to implement OR your data class drive the encoder directly. In this version I have settled for the
latter - you drive the encoder.

Example:
```c++
struct MyDataObject : public IDserialize {
    int someNumber;
    void DeserializeFrom(IDecoder &decoder) {
        // Data does not contain my object
        if (!decoder.BeginObject("MyDataObject")) return;
        auto optIntValue = decoder.ReadIntField("someNumber");
        if (optIntValue.has_value()) {
            someNumber = *optIntValue;
        } else {
            // handle if someNumber is missing
        }
        EndObject();
    }
};
```

This will consume an XML looking like:
```xml
<MyDataObject someNumber="2" />
```

To check if an object has a child present:
```xml
<MyDataObject>
    <MyChild>
        
    </MyChild>
</MyDataObject>
```

You would need to declare two classes/structs (basically you don't need to - but this is probably the better way).
```c++
struct MyDataObject : public IDserialize {
    void DeserializeFrom(IDecoder &decoder) {
        // Data does not contain my object
        if (!decoder.BeginObject("MyDataObject")) return;
        if (decoder.HasObject("MyChild")) {
            child.DeserializeFrom(decoder);
        }
        EndObject();
    }
}

struct MyChildObject : public IDeserialize {
    void DeserializeFrom(IDecoder &decoder) {
        // Data does not contain my object
        if (!decoder.BeginObject("MyChild")) return;
        EndObject();
    }
    
};
```

## XML Parser
The XML Parser is fairly standalone and can be ripped out from the project as-is (more or less). It can be driven in callback mode or as a DOM like parser.
