#include <iostream>
#include <memory>
#include <thread>

#include "factory.hpp"

using namespace hrd28;

struct AnimalParams
{
    AnimalParams(const std::string& animal_name_, const int& num_of_legs_, const std::string& sound_):
    m_animal_name(animal_name_),m_num_of_legs(num_of_legs_),m_sound(sound_) {}
    
    const std::string m_animal_name;
    const int m_num_of_legs;
    const std::string m_sound;
};

class IAnimal
{
public:
    IAnimal(AnimalParams params_): m_params(params_) {}
    
    virtual ~IAnimal() =default;
    
    const std::string& GetAnimalName()
    {
        return m_params.m_animal_name;
    }
    
    const int& GetNumberOfLegs() const
    {
        return m_params.m_num_of_legs;
    }
    
    const std::string& MakeSound()
    {
        return m_params.m_sound;
    }
    
    virtual void Print() =0;
    
private:
    AnimalParams m_params;
};

class Cat : public IAnimal
{
public:
    Cat(AnimalParams params_): IAnimal(params_) {}
    virtual ~Cat() {};

    Cat(const Cat& o_) =delete;
    Cat& operator=(const Cat& o_) =delete;
    
    void Print() override
    {
        std::cout << "The cat name is: " << this->GetAnimalName() << std::endl;
        std::cout << "The cat has " << this->GetNumberOfLegs() << " legs" << std::endl;
        std::cout << "The cat does this sound: " << this->MakeSound() << std::endl << std::endl;
    }
};


class Dog : public IAnimal
{
public:
    Dog(AnimalParams params_): IAnimal(params_) {}
    virtual ~Dog() {};
    
    Dog(const Dog& o_) =delete;
    Dog& operator=(const Dog& o_) =delete;
    
    void Print() override
    {
        std::cout << "The dog name is: " << this->GetAnimalName() << std::endl;
        std::cout << "The dog has " << this->GetNumberOfLegs() << " legs" << std::endl;
        std::cout << "The dog does this sound: " << this->MakeSound() << std::endl << std::endl;
    }
};

template <typename ANIMAL>
static std::unique_ptr<ANIMAL> Create(AnimalParams params_) 
{
    ANIMAL *ret = new ANIMAL(params_);
    ret->Print();

    return std::unique_ptr<ANIMAL>(ret);
}

static void ProdCatMutations()
{
    try
    {
        Singleton<Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->SetKey("cat", Create<Cat>);

        for (int i = 0; i < 10; ++i)
        {
            auto cat(Singleton<Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->Create("cat", AnimalParams("Kitty", i, "Baa!")));
        }
    }
    catch (std::exception& e_)
    {
        std::cout << e_.what() << std::endl;
    }
}

static void ProdDogMutations()
{
    Singleton<Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->SetKey("dog", Create<Dog>);

    for (int i = 0; i < 10; ++i)
    {
        Singleton<Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->Create("dog", AnimalParams("Woofer", i, "Woof!"));
        Singleton<Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->Create("dog", AnimalParams("Sub Woofer", i, "Waf!"));
    }
}

int main()
{
    std::thread cats(ProdCatMutations);
    std::thread dogs(ProdDogMutations);

    cats.join();
    dogs.join();

    return (0);
}
