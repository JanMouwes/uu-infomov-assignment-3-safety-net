namespace Templ8
{
    void TargetsAttractSystem(vector<TargetComponent> targets, vector<SpatialComponent> spatials,
                              vector<SteerComponent> steer)
    {
        // TODO: We need some way to store entities in a sparse way, and to be aware of which entity is where.
        // Just iterating over the array wont work as some entries will be uninitialized because each NewXxx call
        // increments the global NEXT_ENTITY count, but each NewXxx call does not add a component to each component
        // collection.
    }

    void MovementSystem(vector<SpatialComponent> spatials, vector<MovementComponent> movements)
    {
        for (auto spatial : spatials)
        {
        }
    }
}
