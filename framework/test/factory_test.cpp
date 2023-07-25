// gp17d -iquote ~/nadav/projects/final_project/framework/include test/factory_test.cpp
//not need factory.cpp because the class is a template

#include <iostream>

#include "factory.hpp"

class AnimalParams
{
public:
    AnimalParams(std::string animal_name_, int num_of_legs_, std::string sound_):
    m_animal_name(animal_name_),m_num_of_legs(num_of_legs_),m_sound(sound_)
    {}
    
    std::string m_animal_name;
    int m_num_of_legs;
    std::string m_sound;
};

class IAnimal
{
public:
    IAnimal(std::string animal_name_, int num_of_legs_, std::string sound_):
            m_params(AnimalParams(animal_name_, num_of_legs_, sound_))
    {}
    
    std::string& GetAnimalName()
    {
        return m_params.m_animal_name;
    }
    
    int GetNumberOfLegs() const
    {
        return m_params.m_num_of_legs;
    }
    
    std::string& MakeSound()
    {
        return m_params.m_sound;
    }
    
    virtual void Print()
    {}
    
private:
    AnimalParams m_params;
};

class Cat : public IAnimal
{
public:
    Cat(std::string animal_name_, int num_of_legs_, std::string sound_):
        IAnimal(animal_name_, num_of_legs_, sound_)
    {}
    
    void Print()
    {
        std::cout << "The cat name: " << this->GetAnimalName() << std::endl;
        std::cout << "The cat has " << this->GetNumberOfLegs() << " legs" << std::endl;
        std::cout << "The cat do this sound: " << this->MakeSound() << std::endl;
    }
private:
    Cat(const Cat& _other);
    Cat &operator=(const Cat& _other);
};


class Dog : public IAnimal
{
public:
    Dog(std::string animal_name_, int num_of_legs_, std::string sound_):
        IAnimal(animal_name_, num_of_legs_, sound_)
    {}
    
    void Print()
    {
        std::cout << "The dog name: " << this->GetAnimalName() << std::endl;
        std::cout << "The dog has " << this->GetNumberOfLegs() << " legs" << std::endl;
        std::cout << "The dog do this sound: " << this->MakeSound() << std::endl;
    }
private:
    Dog(const Dog& _other);
    Dog &operator=(const Dog& _other);
};


std::unique_ptr<IAnimal> CreateCat(AnimalParams params)
{
    std::unique_ptr<Cat> cat = std::make_unique<Cat>(params.m_animal_name, params.m_num_of_legs, params.m_sound);
    cat->Print();

    return cat;
}

std::unique_ptr<IAnimal> CreateDog(AnimalParams params)
{
    std::unique_ptr<Dog> dog = std::make_unique<Dog>(params.m_animal_name, params.m_num_of_legs, params.m_sound);
    dog->Print();

    return dog;
}


int main()
{
    using namespace hrd28;

    std::string cat_name = "Kitty";
    std::string dog_name = "doggie";
    
    Singleton<hrd28::Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->SetKey("cat", CreateCat);
    Singleton<hrd28::Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->SetKey("dog", CreateDog);
    
    AnimalParams new_params("try", 2, " try");

    AnimalParams main_dog(dog_name, 8, "woaf");
    AnimalParams main_cat(cat_name, 4, "Meao");
    
    Singleton<hrd28::Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->Create("dog", main_dog);
    Singleton<hrd28::Factory<IAnimal, std::string, AnimalParams>>::GetInstance()->Create("cat", main_cat);

    return (0);
}
