#pragma once

#include <list>
#include <memory>
#include <string>

namespace Gadgetron {

    class ACE_Message_Block {

    public:
        ACE_Message_Block() = default;

        explicit ACE_Message_Block(std::string s) : buffer(std::move(s)) {

        }

        const char *rd_ptr() { return buffer.c_str(); };

        virtual ~ACE_Message_Block() {
            if (cont_element) {
                cont_element->release();
            }
        };

        virtual void *release() {
            delete (this); // Seppuku
            return nullptr;
        }

        ACE_Message_Block *cont() { return cont_element; }

        void cont(ACE_Message_Block *ptr) { cont_element = ptr; }


    private:
        ACE_Message_Block *cont_element = nullptr;

    private:
        std::string buffer;
    };

}